The ADS-B plugin uses a modified version of dump1090 that was forked from https://github.com/MalcolmRobb/dump1090

Permission to reuse dump1090 in the ADS-B plugin without applying the GPL license to SDRuno, has been expressly given by the license holder (Oliver Jowett - https://github.com/mutability)

Any modifications to this code must adhere to the GPL license under which it is published.

Known Issues
============
1. If the plugin webserver port (in the plugin Settings panel) is set to the same number that another application (such as Virtual Radar Server) is already using, this will cause the plugin to crash.
2. When using the local webserver, the national flags do not display.
