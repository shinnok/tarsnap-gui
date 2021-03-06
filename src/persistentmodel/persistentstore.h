#ifndef PERSISTENTSTORE_H
#define PERSISTENTSTORE_H

#include <QMutex>
#include <QObject>
#include <QtSql>

#define DEFAULT_DBNAME "tarsnap.db"

/*!
 * \ingroup persistent
 * \brief The PersistentStore is a singleton which interfaces with a database
 * which stores lists of archives, jobs, and journal entries.
 */
class PersistentStore : public QObject
{
    Q_OBJECT

public:
    ~PersistentStore();
    //! Singleton.
    static PersistentStore &instance()
    {
        static PersistentStore instance;
        return instance;
    }
    //! Returns the status of whether the database is initialized or not.
    bool initialized() { return _initialized; }
    //! Returns an empty query attached to the database if it is initialized,
    //! or an unattached query otherwise.
    QSqlQuery createQuery();
    //! Removes the existing database if it is initialized.  Does not lock.
    void purge();

    //! Locks, upgrades the version if it is old, creates a new one otherwise.
    //! Must be run before any other functions in this class.
    bool init();
    //! Closes the database connection (if it exists).  Normally not used by
    //! external classes, with the possible exception of the test suite.
    static void deinit();

public slots:
    //! Locks the database and runs a query.
    bool runQuery(QSqlQuery query);

protected:
    //!@{
    //! Upgrade the database version.
    bool upgradeVersion0();
    bool upgradeVersion1();
    bool upgradeVersion2();
    bool upgradeVersion3();
    bool upgradeVersion4();
    //!@}

private:
    // Yes, a singleton
    explicit PersistentStore(QObject *parent = nullptr);
    PersistentStore(PersistentStore const &);
    void operator=(PersistentStore const &);

    static bool   _initialized;
    static QMutex _mutex;
};

#endif // PERSISTENTSTORE_H
