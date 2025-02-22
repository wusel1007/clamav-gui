#include "infodialog.h"
#include "ui_infodialog.h"

infoDialog::infoDialog(QWidget *parent) : QWidget(parent), ui(new Ui::infoDialog)
{
 QString version = "1.0.8 (QT5)";

    QString infoText =  "<html>";
            infoText += "  <head/>";
            infoText += "    <body>";
            infoText += "      <p align='center'>";
            infoText += "        <span style=' font-size:22px; color:#000080;'>ClamAV-GUI</span>";
            infoText += "      </p>";
            infoText += "      <p align='center'>";
            infoText += "        <span style=' font-size:15px; color:#000000;'><br/>Version " + version + " (2015 - 2025), published unter GPL 3.0</span>";
            infoText += "      </p>";
            infoText += "      <hr/>";
            infoText += "      <table border='0' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;' align='center' cellspacing='2' cellpadding='0'>";
            infoText += "        <tr>";
            infoText += "          <td colspan='7'>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px; font-weight:700; text-decoration: underline;'>Author:</span>";
            infoText += "              <span style=' font-size:14px;'>  Joerg Macedo da Costa Zopes &lt;joerg.zopes@gmx.de&gt;<br/></span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "        </tr>";
            infoText += "        <tr>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px; font-weight:700; text-decoration: underline;'>Translators</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "        </tr>";
            infoText += "        <tr>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <img src='qrc:///icons/icons/dansk.png'/>";
            infoText += "              <span style=' font-size:14px;'> Danish:</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'>Martin Eilssøe &lt;martin.eilsoe@gmail.com&gt;</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <img src='qrc:///icons/icons/spain.png'/>";
            infoText += "              <span style=' font-size:14px;'> Spanish:</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'>Pedro Álamo &lt;palamodz@gmail.com&gt;</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "        </tr>";
            infoText += "        <tr>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <img src='qrc:///icons/icons/germany.png'/>";
            infoText += "              <span style=' font-size:14px;'> German:</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'>translated by the author.</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <img src='qrc:///icons/icons/france.png'/>";
            infoText += "              <span style=' font-size:14px;'> French:</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'>Not be named </span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "        </tr>";
            infoText += "        <tr>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <img src='qrc:///icons/icons/Portugal.png'/>";
            infoText += "              <span style=' font-size:14px;'> Portuguese:</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'>Translated by the author.</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <img src='qrc:///icons/icons/Brasil.png'/>";
            infoText += "              <span style=' font-size:14px;'> Brasilian:</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'>Translated by the author.</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "        </tr>";
            infoText += "        <tr>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <img src='qrc:///icons/icons/GB.png'/>";
            infoText += "              <span style=' font-size:14px;'> English:</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'>Translated by the author.</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <img src='qrc:///icons/icons/italy.png'/>";
            infoText += "              <span style=' font-size:14px;'> Italian:</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'>translated by DeepL</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "        </tr>";
            infoText += "      </table>";
            infoText += "      <hr/>";
            infoText += "      <table border='0' style=' margin-top:20px; margin-bottom:0px; margin-left:20px; margin-right:0px;' cellspacing='2' cellpadding='0'>";
            infoText += "        <tr>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <img src='qrc:///icons/icons/ukraine.png'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'/>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "          <td>";
            infoText += "            <p>";
            infoText += "              <span style=' font-size:14px;'>A special thanks goes to UALinux &lt;main@ualinux.com&gt;.<br/>";
            infoText += "              We were in close contact for more than a week and<br/>";
            infoText += "              with their help I was able to fix some bugs<br/>";
            infoText += "              and implement useful new features.<br/>";
            infoText += "              <br/>";
            infoText += "              Many thanks for that.</span>";
            infoText += "            </p>";
            infoText += "          </td>";
            infoText += "        </tr>";
            infoText += "      </table>";
            infoText += "    </body>";
            infoText += "  </html>";
    ui->setupUi(this);
    ui->creditTextEdit->setHtml(infoText);
}

infoDialog::~infoDialog()
{
    delete ui;
}
