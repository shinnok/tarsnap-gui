#ifndef TARSNAPACCOUNT_H
#define TARSNAPACCOUNT_H

#include "ui_logindialog.h"

#include <QDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>

/*!
 * \ingroup widgets-specialized
 * \brief The TarsnapAccount is a QDialog which displays info about the account
 * and launches network connections to the Tarsnap servers to update that info.
 */
class TarsnapAccount : public QDialog
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestSettingsWidget;
#endif

public:
    //! Constructor.
    explicit TarsnapAccount(QWidget *parent = nullptr);
    ~TarsnapAccount();

public slots:
    //! Update the account info from the Tarsnap servers, optionally showing
    //! the results QTableWidget(s).
    void getAccountInfo(bool displayActivity        = false,
                        bool displayMachineActivity = false);

signals:
    //! The amount of money in the Tarsnap account, and the latest date.
    void accountCredit(qreal credit, QDate date);
    //! The latest machine activity.
    void lastMachineActivity(QStringList activityFields);
    //! Begin tarsnap-keymgmt --print-key-id \<key_filename\>
    void getKeyId(QString key);

protected slots:
    //! Extract the credits from the CSV in the reply; will emit \ref
    //! accountCredit.
    void parseCredit(QString csv);
    //! Extract the machine activity from the CSV in the reply; will emit \ref
    //! lastMachineActivity.
    void parseLastMachineActivity(QString csv);
    //! Format and display a comma-separated-value table in a QTableWidget.
    void displayCSVTable(QString csv, QString title);
    //! Send a network request and wait for a reply.
    QNetworkReply *tarsnapRequest(QString url);
    //! Parse the network reply.
    QByteArray readReply(QNetworkReply *reply, bool warn = false);
    //! Handle an error in the network reply.
    void networkError(QNetworkReply::NetworkError error);
    //! Handle an SSL error in the network reply.
    void sslError(QList<QSslError> errors);

private:
    Ui::loginDialog       _ui;
    QNetworkAccessManager _nam;
    QString               _user;
    QString               _machine;
    quint64               _machineId;

    QMessageBox _popup;
};

#endif // TARSNAPACCOUNT_H
