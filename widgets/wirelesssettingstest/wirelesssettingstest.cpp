#include "wirelesssettingswidget.h"

#include <QApplication>

#include <KUniqueApplication>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KAboutData>
#include <KDialog>

int main(int argc, char **argv)
{
    KAboutData aboutData(QByteArray("testWireless"), 0, ki18n("testWireless"),
                         KDE_VERSION_STRING, ki18n("test"), KAboutData::License_BSD,
                         ki18n("test"));
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    KDialog *settingsdlg = new KDialog();
    settingsdlg->setCaption("Wireless Settings");
    settingsdlg->setButtons( KDialog::Ok | KDialog::Cancel);
    WirelessSettingsWidget *settings = new WirelessSettingsWidget(settingsdlg);
    settings->resize(QSize(640,480));
    settingsdlg->setMainWidget(settings);
    settingsdlg->show();
    
    return app.exec();
}
