#include "manageprofilewidget.h"

#include <QApplication>

#include <KUniqueApplication>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KAboutData>
#include <KDialog>

int main(int argc, char **argv)
{
    KAboutData aboutData(QByteArray("testManageProfiles"), 0, ki18n("testManageProfiles"),
                         KDE_VERSION_STRING, ki18n("test"), KAboutData::License_BSD,
                         ki18n("test"));
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    KDialog *profiledlg = new KDialog();
    profiledlg->setCaption("Manage Profiles");
    profiledlg->setButtons( KDialog::Ok | KDialog::Cancel);
    ManageProfileWidget *profile = new ManageProfileWidget(profiledlg);
    profiledlg->setMainWidget(profile);
    profiledlg->show();
    
    return app.exec();
}

