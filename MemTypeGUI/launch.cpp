#include "launch.h"
#include "strings.h"
#include "stdio.h"
#include "stdint.h"
#include <iostream>
#include <fstream>

Launch::Launch(QObject *parent) :
    QObject(parent),
    m_process(new QProcess(this))
{
}

QString Launch::launchRun(QString cmd)
{
    m_process->start(cmd);
    m_process->waitForFinished();
    return m_process->readAllStandardOutput();
}
QString Launch::getError(void)
{
    return m_process->readAllStandardError();
}

void Launch::writeFile(QString content,QString filename)
{
    std::ofstream myfile;
    myfile.open(filename.toStdString().c_str());
    myfile << content.toStdString();
    myfile.close();
}

QString Launch::getInfo(void)
{
    m_process->start("./hidtool 5");
    m_process->waitForFinished();
    QByteArray data = m_process->readAllStandardOutput();
    //uint8_t inst = data[];
    uint8_t vmaj = data[0];
    uint8_t vmin = data[1];
    uint8_t vpatch = data[2];

    uint16_t size = (data[4] << 8) + (data[3]);
    char dataStr[50];
    sprintf(dataStr,"Version %d.%d.%d;Memory size %u Bytes",vmaj,vmin,vpatch,size);
    return dataStr;
}

