#pragma once

#include <QtCore>
#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QTreeWidget>

#include "core/sound.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void openFile();
    void refreshList();
    void playFile();
    void stopFile();
    void exportFile();
    void exportAllFiles();
private:
    QByteArray createRowWav(int row);
    QByteArray createCurrentRowWav();
    static QTreeWidgetItem *createRow(int gameId, int ff8sndId, const Sound &sound);
    QPushButton *openButton, *playButton, *stopButton, *exportButton, *exportAllButton;
    QTreeWidget *soundsList;
    QList<Sound> _sounds;
    QFile *_fileDat;
};
