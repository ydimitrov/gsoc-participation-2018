#include <QtCore/QDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QUrlQuery>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QThread>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
// #include <QtQuickControls2/QQuickStyle>
#include <QtQml/qqml.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>
#include <QQuickWindow>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <taskmanager.h>

int main(int argc, char *argv[])
{

	QString myname = QString("TaskManager");
	
	QGuiApplication app(argc, argv);

	QCommandLineParser parser;
	parser.addPositionalArgument("port", app.translate("main", "port for binding"));
	parser.addPositionalArgument("secret", app.translate("main", "secret for binding"));
	parser.addHelpOption();
	parser.addVersionOption();
	parser.process(app);
	QStringList positionalArguments = parser.positionalArguments();

	qmlRegisterType<TaskManager>("TaskManager", 1, 0, "TaskManager");

	QQmlApplicationEngine engine;
	if (positionalArguments.length() == 2) {
		int port = positionalArguments.takeFirst().toInt();
		QString secret = positionalArguments.takeFirst();
		QUrl bindingAddress;
		bindingAddress.setScheme(QStringLiteral("ws"));
		bindingAddress.setHost(QStringLiteral("localhost"));
		bindingAddress.setPort(port);
		bindingAddress.setPath(QStringLiteral("/api"));
		QUrlQuery query;
		query.addQueryItem(QStringLiteral("token"), secret);
		bindingAddress.setQuery(query);
		QQmlContext *context = engine.rootContext();
		context->setContextProperty(QStringLiteral("bindingAddress"), bindingAddress);
		qDebug() << "Connect to: " << bindingAddress;
	}
    else { 
        qDebug() << "[ERROR] No port and token specified!";
        return -1;
    }

	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
	if (engine.rootObjects().isEmpty())
		return -1;
	return app.exec();
}