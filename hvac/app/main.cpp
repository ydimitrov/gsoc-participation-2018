/*
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QtCore/QDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QUrlQuery>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickWindow>

#include "translator.h"

#ifdef HAVE_LIBHOMESCREEN
#include <libhomescreen.hpp>
#endif
#ifdef HAVE_QLIBWINDOWMANAGER
#include <qlibwindowmanager.h>
#endif

int main(int argc, char *argv[])
{
    QString myname = QString("HVAC");

    QGuiApplication app(argc, argv);
    app.setApplicationVersion(QStringLiteral("0.1.0"));
    app.setOrganizationDomain(QStringLiteral("automotivelinux.org"));
    app.setOrganizationName(QStringLiteral("AutomotiveGradeLinux"));

    QCommandLineParser parser;
    parser.addPositionalArgument("port", app.translate("main", "port for binding"));
    parser.addPositionalArgument("secret", app.translate("main", "secret for binding"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    QStringList positionalArguments = parser.positionalArguments();

    qmlRegisterType<Translator>("Translator", 1, 0, "Translator");

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();
    QUrl bindingAddress;

    int port;
    QString secret;
    if (positionalArguments.length() == 2) {
        port = positionalArguments.takeFirst().toInt();
        secret = positionalArguments.takeFirst();
        bindingAddress.setScheme(QStringLiteral("ws"));
        bindingAddress.setHost(QStringLiteral("localhost"));
        bindingAddress.setPort(port);
        bindingAddress.setPath(QStringLiteral("/api"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("token"), secret);
        bindingAddress.setQuery(query);
    }
    context->setContextProperty(QStringLiteral("bindingAddress"), bindingAddress);
#ifdef HAVE_QLIBWINDOWMANAGER
    QLibWindowmanager* qwm = new QLibWindowmanager();

    // WindowManager
    if(qwm->init(port,secret) != 0){
        exit(EXIT_FAILURE);
    }
    // Request a surface as described in layers.json windowmanager’s file
    if (qwm->requestSurface(myname) != 0) {
        exit(EXIT_FAILURE);
    }
    // Create an event callbnewack against an event type. Here a lambda is called when SyncDraw event occurs
    qwm->set_event_handler(QLibWindowmanager::Event_SyncDraw, [qwm, myname](json_object *object) {
        fprintf(stderr, "Surface got syncDraw!\n");
        qwm->endDraw(myname);
    });
#endif

#ifdef HAVE_LIBHOMESCREEN
    LibHomeScreen* hs = new LibHomeScreen();

    // HomeScreen
    std::string token = secret.toStdString();
    hs->init(port, token.c_str());
    // Set the event handler for Event_TapShortcut which will activate the surface for windowmanager
    hs->set_event_handler(LibHomeScreen::Event_TapShortcut, [qwm, myname](json_object *object){
        json_object *appnameJ = nullptr;
        if(json_object_object_get_ex(object, "application_name", &appnameJ))
        {
            const char *appname = json_object_get_string(appnameJ);
            if(myname == appname)
            {
                qDebug("Surface %s got tapShortcut\n", appname);
                qwm->activateSurface(myname);
            }
        }
    });
#endif

    engine.load(QUrl(QStringLiteral("qrc:/HVAC.qml")));
    QObject *root = engine.rootObjects().first();
    QQuickWindow *window = qobject_cast<QQuickWindow *>(root);
#ifdef HAVE_QLIBWINDOWMANAGER
    QObject::connect(window, SIGNAL(frameSwapped()), qwm, SLOT(slotActivateSurface()));
#else
    window->resize(1080, 1920 - 218 - 215);
    window->setVisible(true);
#endif

    return app.exec();
}

