#include "wirelesssettingswidget.h"

#include <QApplication>

#include <KUniqueApplication>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KAboutData>
#include <KDialog>

int main(int argc, char **argv)
{
    KAboutData aboutData(QByteArray("testAddProfile"), 0, ki18n("testAddProfile"),
                         KDE_VERSION_STRING, ki18n("test"), KAboutData::License_BSD,
                         ki18n("test"));
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    KDialog *profiledlg = new KDialog();
    profiledlg->setCaption("Wireless Settings");
    profiledlg->setButtons( KDialog::Ok | KDialog::Cancel);
    WirelessSettingsWidget *profile = new WirelessSettingsWidget(profiledlg);
    profile->resize(QSize(640,480));
    profiledlg->setMainWidget(profile);
    profiledlg->show();
    
    return app.exec();
}
