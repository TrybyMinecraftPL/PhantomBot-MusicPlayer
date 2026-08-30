#include <QApplication>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include <cstdlib>
#include <cstdint>
#include <memory>
#include <string>

#define DISCORDPP_IMPLEMENTATION
#include "discordpp.h"

namespace {
constexpr auto kMusicUrl = "https://phantombot.pl/music";
constexpr auto kRedirectUri = "http://127.0.0.1/callback";
constexpr auto kDefaultApplicationId = "0";

uint64_t applicationId() {
    const char* value = std::getenv("DISCORD_APPLICATION_ID");
    if (!value || std::string(value).empty()) return 0;
    try {
        return std::stoull(value);
    } catch (...) {
        return 0;
    }
}

class MusicPlayerWindow final : public QWidget {
public:
    MusicPlayerWindow() {
        setWindowTitle(QStringLiteral("PhantomBot Music Player"));
        setMinimumSize(560, 390);
        setStyleSheet(R"(
            QWidget { background: #111827; color: #f9fafb; font-family: Arial; }
            QLabel#title { font-size: 30px; font-weight: 700; color: #ffffff; }
            QLabel#subtitle { font-size: 14px; color: #cbd5e1; }
            QLabel#status { background: #1f2937; border: 1px solid #374151; border-radius: 8px; padding: 10px; color: #cbd5e1; }
            QPushButton { border: 0; border-radius: 8px; padding: 12px 18px; font-size: 14px; font-weight: 600; }
            QPushButton#music { background: #7c3aed; color: white; }
            QPushButton#music:hover { background: #8b5cf6; }
            QPushButton#discord { background: #5865f2; color: white; }
            QPushButton#discord:hover { background: #7289da; }
            QPushButton#secondary { background: #374151; color: #f9fafb; }
            QPushButton#secondary:hover { background: #4b5563; }
        )");

        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(42, 36, 42, 36);
        root->setSpacing(16);

        auto* brand = new QLabel(QStringLiteral("PHANTOMBOT"));
        brand->setStyleSheet("color:#a78bfa; font-size:12px; font-weight:700; letter-spacing:2px;");
        root->addWidget(brand);

        auto* title = new QLabel(QStringLiteral("PhantomBot Music Player"));
        title->setObjectName("title");
        root->addWidget(title);

        auto* subtitle = new QLabel(QStringLiteral(
            "Muzyka, społeczność i Discord w jednym miejscu.\n"
            "Otwórz odtwarzacz PhantomBot i zostań w kontakcie ze swoją ekipą."));
        subtitle->setObjectName("subtitle");
        subtitle->setWordWrap(true);
        root->addWidget(subtitle);

        root->addSpacing(8);

        auto* statusTitle = new QLabel(QStringLiteral("Status Discord"));
        statusTitle->setStyleSheet("font-size:12px; font-weight:700; color:#94a3b8;");
        root->addWidget(statusTitle);

        status_ = new QLabel(QStringLiteral("Niepołączono — kliknij „Połącz z Discordem”, aby rozpocząć."));
        status_->setObjectName("status");
        status_->setWordWrap(true);
        root->addWidget(status_);

        root->addStretch();

        auto* buttons = new QHBoxLayout();
        buttons->setSpacing(12);

        auto* music = new QPushButton(QStringLiteral("Posłuchaj Muzyki"));
        music->setObjectName("music");
        music->setCursor(Qt::PointingHandCursor);
        connect(music, &QPushButton::clicked, this, [] {
            QDesktopServices::openUrl(QUrl(QString::fromUtf8(kMusicUrl)));
        });
        buttons->addWidget(music);

        discordButton_ = new QPushButton(QStringLiteral("Połącz z Discordem"));
        discordButton_->setObjectName("discord");
        discordButton_->setCursor(Qt::PointingHandCursor);
        connect(discordButton_, &QPushButton::clicked, this, &MusicPlayerWindow::authorize);
        buttons->addWidget(discordButton_);
        root->addLayout(buttons);

        auto* footer = new QLabel(QStringLiteral("PhantomBot Music Player • Social SDK integration"));
        footer->setStyleSheet("color:#64748b; font-size:11px;");
        footer->setAlignment(Qt::AlignCenter);
        root->addWidget(footer);

        // The SDK processes its event queue from the application's main loop.
        auto* sdkTimer = new QTimer(this);
        connect(sdkTimer, &QTimer::timeout, this, [] { discordpp::RunCallbacks(); });
        sdkTimer->start(16);

        client_ = std::make_unique<discordpp::Client>();
        client_->SetStatusChangedCallback([this](discordpp::Client::Status status, discordpp::Client::Error error, int errorCode) {
            QMetaObject::invokeMethod(this, [this, status] {
                status_->setText(QStringLiteral("Discord SDK: %1").arg(static_cast<int>(status)));
            }, Qt::QueuedConnection);
        });
        client_->Connect();
    }

private:
    void authorize() {
        const auto id = applicationId();
        if (id == 0) {
            QMessageBox::warning(this, QStringLiteral("Brak Application ID"),
                QStringLiteral("Ustaw zmienną środowiskową DISCORD_APPLICATION_ID na Application ID swojej aplikacji Discord, a następnie uruchom program ponownie."));
            return;
        }

        discordButton_->setEnabled(false);
        status_->setText(QStringLiteral("Otwieram autoryzację Discord w przeglądarce…"));

        verifier_ = client_->CreateAuthorizationCodeVerifier();
        discordpp::AuthorizationArgs args;
        args.SetClientId(id);
        args.SetScopes(discordpp::Client::GetDefaultPresenceScopes());
        args.SetCodeChallenge(verifier_.Challenge());

        client_->Authorize(std::move(args), [this, id](discordpp::ClientResult result,
                                                       std::string code,
                                                       std::string redirectUri) {
            if (!result.Successful()) {
                finishAuth(QStringLiteral("Autoryzacja anulowana lub zakończona błędem."));
                return;
            }

            status_->setText(QStringLiteral("Wymieniam kod autoryzacyjny na token…"));
            client_->GetToken(id, code, verifier_.Verifier(), redirectUri,
                [this](discordpp::ClientResult tokenResult,
                       std::string /*accessToken*/,
                       std::string /*refreshToken*/,
                       discordpp::AuthorizationTokenType /*tokenType*/,
                       int32_t /*expiresIn*/,
                       std::string /*scopes*/) {
                    if (!tokenResult.Successful()) {
                        finishAuth(QStringLiteral("Nie udało się uzyskać tokenu Discord."));
                        return;
                    }
                    finishAuth(QStringLiteral("Połączono z Discordem. PhantomBot jest gotowy."));
                });
        });
    }

    void finishAuth(const QString& message) {
        QMetaObject::invokeMethod(this, [this, message] {
            status_->setText(message);
            discordButton_->setEnabled(true);
        }, Qt::QueuedConnection);
    }

    std::unique_ptr<discordpp::Client> client_;
    discordpp::AuthorizationCodeVerifier verifier_{discordpp::AuthorizationCodeVerifier::nullobj};
    QLabel* status_{};
    QPushButton* discordButton_{};
};
} // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("PhantomBot Music Player"));
    app.setOrganizationName(QStringLiteral("PhantomBot"));
    MusicPlayerWindow window;
    window.show();
    return app.exec();
}
