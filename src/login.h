#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>

class QLineEdit;
class QLabel;
class QCheckBox;

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);
    void reinitialiser();

private slots:
    void onConnexion();
    void onAfficherMotDePasse(bool visible);

private:
    QLineEdit* leLogin;
    QLineEdit* leMotDePasse;
    QLabel* lblErreur;
    QCheckBox* chkVisible;
};

#endif // LOGIN_H
