#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filedownloader.h"

#include <QTimer>
#include <QtXml>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::print(const QString &text)
{
    ui->outText->append(text);
}

static QString spritedbData;

void MainWindow::on_fetchConv_btn_clicked()
{
    ui->outText->clear();

    print("Downloading spritedata.xml from \"https://nsmbhd.net/spritexml.php\"");
    QUrl spritedbUrl("https://nsmbhd.net/spritexml.php");
    FileDownloader* spritedbDownloader = new FileDownloader(spritedbUrl, this);

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(spritedbDownloader, &FileDownloader::downloaded, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(10*1000);
    loop.exec();

    spritedbData = spritedbDownloader->downloadedData();

    if(spritedbData == "")
    {
        print("Download timed out or data returned empty.");
        return;
    }

    print("Download succeeded, proceeding with the conversion.");
    convertSpriteFormatToClassFormat();
}

void MainWindow::convertSpriteFormatToClassFormat()
{
    QVector<int> classIDForSprite(326);
    QFile classIDForSpriteFile(qApp->applicationDirPath() + "/classIDforSprite.txt");
    if (!classIDForSpriteFile.open(QIODevice::ReadOnly))
    {
        print("Failed to open classIDforSprite.txt document.");
        return;
    }

    QTextStream classIDForSpriteTextStream(&classIDForSpriteFile);
    while (!classIDForSpriteTextStream.atEnd())
    {
        QString line = classIDForSpriteTextStream.readLine();
        classIDForSprite[line.split('=')[0].toInt()] = line.split('=')[1].toInt();
    }
    classIDForSpriteFile.close();

    print("Counting duplicated classes.");
    //Count duplicated classes
    QVector<int> classIDsFound;
    QVector<int> spritesToRemove;
    for(int i = 0; i <= 325; i++)
    {
        if(classIDsFound.contains(classIDForSprite[i]))
            spritesToRemove.append(i);
        classIDsFound.append(classIDForSprite[i]);
    }

    QDomDocument spritedbXml;
    spritedbXml.setContent(spritedbData);

    QDomElement root = spritedbXml.firstChildElement();

    print("Renaming sprites to classes.");
    //Rename sprites to classes
    root.firstChildElement("sprites").setTagName("classes");
    QDomNodeList sprites = root.elementsByTagName("sprite");
    for(int i = 0; i < sprites.count(); i++)
        sprites.at(i).toElement().setTagName("class");
    QDomNodeList classes = root.elementsByTagName("class");

    print("Removing duplicated classes.");
    //Remove duplicated classes
    for(int spriteToRemove : spritesToRemove)
    {
        for(int i = 0; i < classes.count(); i++)
        {
            QDomNode _class = classes.at(i);
            int id = _class.toElement().attribute("id").toInt();
            if(id == spriteToRemove)
                _class.parentNode().removeChild(_class);
        }
    }

    print("Remapping IDs from sprite IDs to class IDs.");
    //Remap IDs from sprite IDs to class IDs
    for(int i = 0; i < classes.count(); i++)
    {
        int id = classes.at(i).toElement().attribute("id").toInt();
        classes.at(i).toElement().attributeNode("id").setValue(QString::number(classIDForSprite[id]));
    }

    print("Writing converted XML to file.");
    //Write converted XML to file
    QFile outXmlFile(qApp->applicationDirPath() + "/classdata.xml");
    if(!outXmlFile.open(QIODevice::WriteOnly))
    {
        print( "Failed to open file for writing." );
        return;
    }
    QTextStream stream(&outXmlFile);
    stream << spritedbXml.toString();
    outXmlFile.close();
    print("The file was successfully converted and saved.");
}
