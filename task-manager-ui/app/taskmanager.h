#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>
#include <QtCore>
#include "qafbwebsocketclient.h"
#include "procinfo.h"

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

class TaskManager : public QObject
{
	Q_OBJECT

public:
    explicit TaskManager(QObject* parent = nullptr);

    Q_INVOKABLE void open(const QUrl& url);
    QTimer *timer;

signals:
	void updateProcess(const QString& cmd_, int tid_, int euid_, double scpu_, double ucpu_, double resident_memory_, const QString& state_);
	void addProcess(const QString& cmd_, int tid_, int euid_, double scpu_, double ucpu_, double resident_memory_, const QString& state_);
	void removeProcess(int tid_);

private slots:
	void onClientConnected();
	void callService();

private:
	QAfbWebsocketClient m_client;
	// QVector<ProcInfo> m_procinfos;
	std::vector<ProcInfo> m_procinfos;
};

#endif // TASKMANAGER_H
