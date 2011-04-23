#include <KStandardDirs>
#include <solid/control/modemmanager.h>

#include "../config/mobileconnectionwizard.h"

class Bluetooth: public QObject
{
Q_OBJECT
public:
    Bluetooth(const QString bdaddr, const QString device);
    ~Bluetooth();
    static void saveConnection(Knm::Connection *con);

private Q_SLOTS:
    void modemAdded(const QString &udi);

private:
    QString mBdaddr;
    QString mDevice;
    MobileConnectionWizard *mobileConnectionWizard;

    QString deviceName();
};
