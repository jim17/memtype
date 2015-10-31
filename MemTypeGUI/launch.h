#ifndef LAUNCH_H
#define LAUNCH_H

#include <QObject>
#include <QProcess>

class Launch : public QObject
{

    Q_OBJECT

public:
    explicit Launch(QObject *parent = 0);
    Q_INVOKABLE QString launchRun(QString);
    Q_INVOKABLE QString getError(void);
    Q_INVOKABLE void writeFile(QString content,QString filename);
    Q_INVOKABLE QString getInfo(void);

private:
    QProcess *m_process;
};

#endif // LAUNCH_H
