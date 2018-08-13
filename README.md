	 	 	
GSoC 2018 Automotive Grade Linux (AGL)
Yordan Dimitrov







##About the project

The tasks that I had while working for the AGL community was to extended an already deployed application as well as develop a full stack application on my own. The first application that I had to extend was an automotive HVAC control dashboard to add an LED functionality. The next task that I was given was to construct a Task Manager application that would display all of the processes currently running on a target board (Raspberry Pie). This consisted of 2 subtasks: developing a backend service that would collect the needed information and a frontend to display the information in a user friendly way.


##About the organization

Automotive Grade Linux (AGL) is a collaborative open source project that is bringing together automakers, suppliers and technology companies to build a Linux-based, open software platform for automotive applications that can serve as the de facto industry standard. Adopting a shared platform across the industry reduces fragmentation and allows automakers and suppliers to reuse the same code base, leading to rapid innovation and faster time-to-market for new products. Automotive Grade Linux is a Project at The Linux Foundation.

##Application description

1. HVAC LED implementation

#About

As the name suggests, this module is used to control the air conditioning, heating and cooling of a vehicle. The application is part of the OS developed by AGL. My task was to devise an implementation that will work along with an LED that will shift its colour gradually from blue to red depending on the temperature set via the temperature toggles. Given that one toggle was set to the highest temperature and the other one - to the lowest the LED was required to emit a mixture of red and blue corresponding to both inputs. 

Another smaller feature that I developed was to allow the use of a configuration file to input LED path values without having to recompile the entire widget. 

#Dependencies
*json-c >= 0.11.99 (for native build)
*afb-daemon (for native build)
*cmake (for native build)
*Qt and QML headers (for native build)
*In its current compilation setup the widget is only runnable under the AGL framework, hence the above packages would be unnecessary 
*Raspberry compilation SDK (for target build)
	
#Compilation
```
cd /package
source SDK from previous step
qmake
make
```

1. Task manager application

	1.Architecture
The structure embodies a three-tier architecture. The leading component of the application is the middleware module which calls the backend service to gather the required information from /proc and distribute it to the frontend. This component is also responsible for determining what action needs to be taken for every single process, be it update, add, or remove it from frontend visualization. The diagram below shows the architecture. 



	1.Task manager service

#About

The backend service part of the application is responsible for collecting the information that will be required. These include process name, process ID, user, CPU% in kernel mode, CPU% in user mode, memory utilization and state. All of the needed data was extracted from the /proc system folder through the use of the external library procps. The acquired data is then sent to the frontend via websocket transfer. This part of the application also establishes the subscription to the AGL framework in order to receive calls. 
		
#Dependencies
*json-c >= 0.11.99 (for native build)
*libprocps (for native build)
*afb-daemon (for native build)
*gcc compiler (for native build)
*The backend service is compilable for a target board (Raspberry Pie) via the existing SDK. If using the SDK the previous packages are not required.

		
#Compilation
```
cd /build
cmake ../
make
```

	1.Task manager UI

#About

The frontend part of the application is responsible for creating the binding (middleware) to the service part as well as to display in a user friendly way the information collected from the /proc folder. Each function that is executed in the QML is exposed to the middleware. On the next figure it can be seen how the frontend looks like populated with current processes.



#Dependencies
*Qt and QML headers (for native build)
*cmake (for native build)
*afb-daemon (for native build)
*g++ compiler >= 4.9.9
		
#Compilation
```
cd /build
cmake ../
make
```

#Running
```
cd /task-manager-service-cmake/build/package
afb-daemon --binding afb-taskmanager.so --port 1234 --token 'HELLO'
in a new terminal: cd /task-manager-ui/build/app
./taskmanager 1234 HELLO
```

##Future work

In terms of my participation in the GSoC program and working with the AGL community, there are some tasks which I could not begin due to time constraints. As future work I identify starting work on the tasks in question some of which are:
Extending the functionality of the Task manager to kill/start applications from the UI
Developing a console application for the AGL framework.
Implementing a CAN Monitor application that uses the existing CAN APIs.

##Acknowledgements

My participation in GSoC was extensively supported by Jan-Simon Möller, his expertise has proven very helpful in many cases. I would also like to thank Loïc Collignon and Romain Forlot from IoT.bzh for their thorough explanations and suggestions which have been extremely useful. 

