#include <QJsonArray>
#include <QJsonObject>
#include <QtDebug>
#include <QString>
#include <unistd.h>
#include <iostream>
#include <QtCore>
#include "taskmanager.h"

TaskManager::TaskManager(QObject* parent) : QObject(parent) {
	qDebug() << "Inside contructor";
	connect(&m_client, SIGNAL(connected()), this, SLOT(onClientConnected()));
}

void TaskManager::open(const QUrl &url) {
    qDebug() << "Entered open";
    m_client.open(url);
}

void TaskManager::onClientConnected() {
	qDebug() << "Entered onClientConnected";

	timer = new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(callService()));
	timer->start(3000);
}


void TaskManager::callService(){
	qDebug() << "Entered callService";
	m_client.call("taskmanager", "get_process_list", QJsonValue(), [this](bool r, const QJsonValue& val) {
		// QVector<ProcInfo> procs;
		std::vector<ProcInfo> procs;
		
		if (r) {

			QJsonObject ret_val = val.toObject();
			QJsonObject response = ret_val["response"].toObject();
			QJsonArray processes = response["processes"].toArray();

			for(auto it = processes.constBegin(); it != processes.constEnd(); ++it)
			{
				ProcInfo Proc(it->toObject());
				procs.push_back(Proc);
			}
		}

		int flag;
		if(m_procinfos.empty()){
			for(auto it = procs.begin(); it != procs.end(); ++it){ // if m_procinfos is empty then this is first call
				emit addProcess(it->cmd(), it->tid(), it->euid(), it->scpu(), it->ucpu(), it->resident_memory(), it->state()); 
			}			
		} else {
			for(auto it = procs.begin(); it != procs.end(); ++it){						// loop through procs, it = procs element (ProcInfo obj)
				flag = 0;
				for(auto it2 = m_procinfos.begin(); it2 != m_procinfos.end(); ++it2){	// loop through m_procinfos, it2 m_procinfos element (ProcInfo obj)
					// if(*it == *it2){  // if the same ID exists in both vectors
					if(it->tid() == it2->tid()){
						// qDebug() << "Compares TIDs" << it->tid() << "and" << it2->tid();
						if(it->cmd() == it2->cmd()){ // if the name is still the same
							// qDebug() << "Compares CMDs" << it->cmd() << "and" << it2->cmd();
							if(!(it == it2)){ // if the same process has new data
								// qDebug () << "Compares if two obj are the same";
								emit updateProcess(it->cmd(), it->tid(), it->euid(), it->scpu(), it->ucpu(), it->resident_memory(), it->state());
								// qDebug() << "processes was updated in QML";
								m_procinfos.erase(it2);
								// qDebug() << "process was removed from m_procinfos";
								flag = 1;
								// qDebug() << "obj comparison is finished";
								break;
							}
						} else { // process in m_procinfos has died and a new one has its ID
							qDebug() << "The process ID has been reused for another process";
							emit updateProcess(it->cmd(), it->tid(), it->euid(), it->scpu(), it->ucpu(), it->resident_memory(), it->state());
							m_procinfos.erase(it2);
							flag = 1;
						}
					}
				}
				if(flag == 0){ // if no ID was found in old vector; that means it's a new process
					qDebug() << it->cmd() << " process has been added";
					emit addProcess(it->cmd(), it->tid(), it->euid(), it->scpu(), it->ucpu(), it->resident_memory(), it->state()); 
				}
			}
			for(auto it2 = m_procinfos.begin(); it2 != m_procinfos.end(); ++it2){ // remaining processes in m_procinfos are all dead
				qDebug() << "Dead processes are removed";
				qDebug() << it2->cmd();
				emit removeProcess(it2->tid());
			}
		}
		m_procinfos = procs;
	});
}