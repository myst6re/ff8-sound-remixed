#include "mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include "core/io/audiofmt.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <Mmsystem.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), _fileDat(nullptr)
{
    setWindowTitle(tr("FF8 Sound Remixed"));

    openButton = new QPushButton(tr("Open"), this);
    playButton = new QPushButton(tr("Play"), this);
    stopButton = new QPushButton(tr("Stop"), this);
    exportButton = new QPushButton(tr("Export"), this);
    exportAllButton = new QPushButton(tr("Export All"), this);
    
#ifndef Q_OS_WIN
    playButton->hide();
    stopButton->hide();
#endif
    
    soundsList = new QTreeWidget(this);
    soundsList->setUniformRowHeights(true);
    soundsList->setHeaderLabels(QStringList() << tr("Game ID") << tr("Clip ID") << tr("Clip size") << tr("Sampl. freq.") << tr("Is Looping?") << tr("Channels") << tr("Is Compressed?"));
    soundsList->setIndentation(0);
    soundsList->setSortingEnabled(true);
    soundsList->sortByColumn(0, Qt::AscendingOrder);
    soundsList->setAutoScroll(false);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(playButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(exportButton);
    buttonLayout->addWidget(exportAllButton);
    buttonLayout->addStretch(1);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(openButton, 0, Qt::AlignLeft);
    layout->addWidget(soundsList, 1);
    layout->addLayout(buttonLayout);
    
    connect(openButton, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(playButton, &QPushButton::clicked, this, &MainWindow::playFile);
    connect(soundsList, &QTreeWidget::itemDoubleClicked, this, &MainWindow::playFile);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::stopFile);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportFile);
    connect(exportAllButton, &QPushButton::clicked, this, &MainWindow::exportAllFiles);
    
    setMinimumSize(640, 480);
}

MainWindow::~MainWindow() {}

void MainWindow::openFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open"), "", tr("FF8 Sound file (*.dat)"));
    if (path.isNull()) {
        return;
    }
    
    QFileInfo fileInfo(path);

    QFile file(path.first(path.size() - fileInfo.suffix().size()) + "fmt");
    if (! file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open FMT file: %1 (%2)")
                                                    .arg(file.errorString(), file.fileName()));
        return;
    }

    AudioFmt audioFmt(&file);
    if (! audioFmt.readAll(_sounds)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open FMT file: Invalid format (%1)")
                                                    .arg(file.fileName()));
    }
    
    refreshList();
    
    if (_fileDat != nullptr) {
        delete _fileDat;
    }
    
    _fileDat = new QFile(path);
}

void MainWindow::refreshList()
{
    QList<QTreeWidgetItem *> items;
    
    int gameId = 0, ff8sndId = 0;
    for (const Sound &sound: _sounds) {
        if (sound.isValid()) {
            items.append(createRow(gameId, ff8sndId, sound));
            ff8sndId += 1;
        }
        gameId += 1;
    }
    
    soundsList->blockSignals(true);
    soundsList->clear();
    soundsList->addTopLevelItems(items);
    soundsList->resizeColumnToContents(0);
    soundsList->resizeColumnToContents(1);
    soundsList->blockSignals(false);
    if (! items.isEmpty()) {
        soundsList->setCurrentItem(items.first());
    }
}

QTreeWidgetItem *MainWindow::createRow(int gameId, int ff8sndId, const Sound &sound)
{
    QStringList columns;
    
    columns.append(QString("%1").arg(gameId, 5));
    columns.append(QString("%1").arg(ff8sndId, 5));
    columns.append(QString("%1").arg(sound.dataLength(), 10));
    columns.append(QString::number(sound.format().nSamplesPerSec));
    columns.append(sound.isLooping() ? tr("Yes") : QString());
    columns.append(QString::number(sound.format().nChannels));
    columns.append(sound.format().wFormatTag == 2 ? tr("Yes") : QString());

    QTreeWidgetItem *item = new QTreeWidgetItem(columns);
    item->setTextAlignment(0, Qt::AlignRight);
    item->setTextAlignment(1, Qt::AlignRight);
    item->setTextAlignment(2, Qt::AlignRight);
    item->setTextAlignment(3, Qt::AlignRight);
    item->setTextAlignment(5, Qt::AlignRight);
    item->setData(0, Qt::UserRole, gameId);
    
    return item;
}

QByteArray MainWindow::createCurrentRowWav()
{
    if (_fileDat == nullptr) {
        return QByteArray();
    }
    
    QTreeWidgetItem *item = soundsList->currentItem();
    
    if (item == nullptr) {
        return QByteArray();
    }
    
    int gameId = item->data(0, Qt::UserRole).toInt();
    
    if (gameId >= _sounds.size() || gameId < 0) {
        return QByteArray();
    }
    
    if (! _fileDat->isOpen() && ! _fileDat->open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open DAT file: %1 (%2)")
                                                    .arg(_fileDat->errorString(), _fileDat->fileName()));
        return QByteArray();
    }
    
    const Sound &sound = _sounds.at(gameId);
    QByteArray wav = sound.toWav(_fileDat);
    
    if (wav.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot convert to WAV: %1 (%2)")
                                                    .arg(_fileDat->errorString(), _fileDat->fileName()));
    }
    
    return wav;
}

void MainWindow::playFile()
{
    QByteArray wav = createCurrentRowWav();
    
    if (wav.isEmpty()) {
        return;
    }

#ifdef Q_OS_WIN
    if (! sndPlaySoundA(wav.constData(), SND_MEMORY | SND_ASYNC)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot play WAV file: %1")
                                                    .arg(_fileDat->fileName()));
        return;
    }
#endif
}

void MainWindow::stopFile()
{
#ifdef Q_OS_WIN
    sndPlaySoundA(nullptr, SND_MEMORY | SND_ASYNC);
#endif
}

void MainWindow::exportFile()
{
    QByteArray wav = createCurrentRowWav();
    
    if (wav.isEmpty()) {
        return;
    }
    
    QString path = QFileDialog::getSaveFileName(this, tr("Export"), "", tr("WAV file (*.wav)"));
    if (path.isNull()) {
        return;
    }
    
    QFile outputFile(path);
    if (! outputFile.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open target file: %1 (%2)")
                                                    .arg(outputFile.errorString(), outputFile.fileName()));
        return;
    }
    
    outputFile.write(wav);
    outputFile.close();
}

void MainWindow::exportAllFiles()
{
    if (! _fileDat->isOpen() && ! _fileDat->open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open DAT file: %1 (%2)")
                                                    .arg(_fileDat->errorString(), _fileDat->fileName()));
        return;
    }
    
    QString path = QFileDialog::getExistingDirectory(this, tr("Export All"), "");
    if (path.isNull()) {
        return;
    }
    
    QDir outputDir(path);
    
    int gameId = 0;
    for (const Sound &sound: _sounds) {
        if (! sound.isValid()) {
            gameId += 1;
            continue;
        }

        QByteArray wav = sound.toWav(_fileDat);
        
        if (wav.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Cannot convert to WAV: %1 (%2)")
                                                        .arg(_fileDat->errorString(), _fileDat->fileName()));
            return;
        }
        
        QFile outputFile(outputDir.filePath(QString("%1.wav").arg(gameId)));
        if (! outputFile.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
            QMessageBox::warning(this, tr("Error"), tr("Cannot open target file: %1 (%2)")
                                                        .arg(outputFile.errorString(), outputFile.fileName()));
            return;
        }
        
        outputFile.write(wav);
        outputFile.close();
        
        gameId += 1;
    }
}
