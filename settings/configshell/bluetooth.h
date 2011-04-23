#include <KStandardDirs>

#ifdef COMPILE_MODEM_MANAGER_SUPPORT
#include <solid/control/modemmanager.h>
#endif

#include "../config/mobileconnectionwizard.h"

class Bluetooth: public QObject
{
Q_OBJECT
public:
    static void saveConnection(Knm::Connection *con);
#ifdef COMPILE_MODEM_MANAGER_SUPPORT
    Bluetooth(const QString bdaddr, const QString device);
    ~Bluetooth();


private Q_SLOTS:
    void modemAdded(const QString &udi);
#endif
private:
    QString mBdaddr;
    QString mDevice;
    MobileConnectionWizard *mobileConnectionWizard;

    QString deviceName();
};
