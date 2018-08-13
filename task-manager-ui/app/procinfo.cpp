#include "procinfo.h"
#include <QJsonObject>
#include <QString>

ProcInfo::ProcInfo(const QJsonObject& obj)
{
	m_cmd = obj["cmd"].toString();
	m_tid = obj["tid"].toInt();
	m_euid = obj["euid"].toInt();
	m_scpu = obj["scpu"].toDouble();
	m_ucpu = obj["ucpu"].toDouble();
	m_resident_memory = obj["resident_mem"].toDouble();
	m_state = obj["state"].toString();
}

bool ProcInfo::operator==(const ProcInfo& o)
{
	if (&o == this) return true;
	return // Adapt as require
		m_tid == o.m_tid;
}

QString ProcInfo::cmd() const
{
	return m_cmd;
}

int ProcInfo::tid() const
{
	return m_tid;
}

int ProcInfo::euid() const
{
	return m_euid;
}

double ProcInfo::scpu() const
{
	return m_scpu;
}

double ProcInfo::ucpu() const
{
	return m_ucpu;
}

double ProcInfo::resident_memory() const
{
	return m_resident_memory;
}

QString ProcInfo::state() const
{
	return m_state;
}

/*
 * TODO: either it's a member operator that take one parameter (see above), either it's a global function which take two parameters.
bool operator==(const ProcInfo &obj) {
	ProcInfo obj2(jobj); 
	if(this.m_cmd == obj.m_cmd &&
	   this.m_tid == obj.m_tid &&
	   this.m_euid == obj.m_euid &&
	   this.m_scpu == obj.m_scpu &&
	   this.m_ucpu == obj.m_ucpu &&
	   this.m_resident_memory == obj.m_resident_memory &&
	   this.m_state == obj.m_state) {
		return true;
	} else {
		return false;
	}
}
*/
