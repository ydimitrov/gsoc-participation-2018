#pragma once

#include <QString>
#include <QJsonObject>

class ProcInfo
{
public:
    QString cmd() const;
    int tid() const;
    int euid() const;
    double scpu() const;
    double ucpu() const;
    double resident_memory() const;
    QString state() const;

    explicit ProcInfo() = default;
    explicit ProcInfo(const ProcInfo&) = default;
    explicit ProcInfo(ProcInfo&&) = default;
    ~ProcInfo() = default;

    explicit ProcInfo(const QJsonObject& obj);

    ProcInfo& operator=(const ProcInfo&) = default;
    bool operator==(const ProcInfo& o);

private:
	QString m_cmd;
	int m_tid;
	int m_euid;
	double m_scpu;
	double m_ucpu;
	double m_resident_memory;
	QString m_state;
};
