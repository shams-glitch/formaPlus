#include "login.h"
#include "authentification.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QString::fromUtf8("FormaPlus — Connexion"));
    setModal(true);
    setFixedSize(420, 360);
    setStyleSheet(QString::fromUtf8(
        "QDialog { background: #F4F6F8; }"
        "QLabel#lblMarque { color: #0F2744; font-size: 22px; font-weight: 700; }"
        "QLabel#lblSous { color: #5D6D7E; font-size: 12px; }"
        "QLabel#lblErreur { color: #C0392B; font-size: 12px; }"
        "QLineEdit { padding: 8px; min-height: 26px; border: 1px solid #D5DDE5; "
        "border-radius: 4px; background: #FFFFFF; }"
        "QPushButton#btnOk { background: #0F2744; color: white; padding: 8px 16px; "
        "border: none; border-radius: 4px; }"
        "QPushButton#btnOk:hover { background: #1A3A5C; }"
        "QPushButton#btnQuit { background: transparent; color: #5D6D7E; border: none; }"
    ));

    auto* marque = new QLabel(QString::fromUtf8("FormaPlus"), this);
    marque->setObjectName(QStringLiteral("lblMarque"));
    auto* sous = new QLabel(QString::fromUtf8("GCentreFormation — authentification applicative"), this);
    sous->setObjectName(QStringLiteral("lblSous"));
    sous->setWordWrap(true);

    leLogin = new QLineEdit(this);
    leLogin->setPlaceholderText(QString::fromUtf8("Identifiant (ex. formateur)"));
    leMotDePasse = new QLineEdit(this);
    leMotDePasse->setPlaceholderText(QString::fromUtf8("Mot de passe"));
    leMotDePasse->setEchoMode(QLineEdit::Password);

    chkVisible = new QCheckBox(QString::fromUtf8("Afficher le mot de passe"), this);
    connect(chkVisible, &QCheckBox::toggled, this, &LoginDialog::onAfficherMotDePasse);

    lblErreur = new QLabel(this);
    lblErreur->setObjectName(QStringLiteral("lblErreur"));
    lblErreur->setWordWrap(true);

    auto* btnOk = new QPushButton(QString::fromUtf8("Connexion"), this);
    btnOk->setObjectName(QStringLiteral("btnOk"));
    btnOk->setDefault(true);
    auto* btnQuit = new QPushButton(QString::fromUtf8("Quitter"), this);
    btnQuit->setObjectName(QStringLiteral("btnQuit"));
    connect(btnOk, &QPushButton::clicked, this, &LoginDialog::onConnexion);
    connect(btnQuit, &QPushButton::clicked, this, &QDialog::reject);

    auto* boutons = new QHBoxLayout();
    boutons->addWidget(btnQuit);
    boutons->addStretch();
    boutons->addWidget(btnOk);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(10);
    layout->addWidget(marque);
    layout->addWidget(sous);
    layout->addSpacing(8);
    layout->addWidget(leLogin);
    layout->addWidget(leMotDePasse);
    layout->addWidget(chkVisible);
    layout->addWidget(lblErreur);
    layout->addStretch();
    layout->addLayout(boutons);
}

void LoginDialog::reinitialiser()
{
    leMotDePasse->clear();
    chkVisible->setChecked(false);
    leMotDePasse->setEchoMode(QLineEdit::Password);
    lblErreur->clear();
    leLogin->setFocus();
}

void LoginDialog::onAfficherMotDePasse(bool visible)
{
    leMotDePasse->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
}

void LoginDialog::onConnexion()
{
    Authentification auth;
    if (!auth.authentifier(leLogin->text(), leMotDePasse->text())) {
        lblErreur->setText(auth.lastError());
        leMotDePasse->clear();
        leMotDePasse->setFocus();
        return;
    }
    accept();
}
