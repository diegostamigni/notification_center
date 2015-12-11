# notification_center
### MIT License

This is a simple implementation of a [NSNotificationCenter](https://developer.apple.com/library/mac/documentation/Cocoa/Reference/Foundation/Classes/NSNotificationCenter_Class) using the C++ (11/14) STL APIs. Pull requests are appreciated.

#####Usage:

```c++
#include "notification_center.h"
#include <iostream>

struct Test : ds::Observable {
	// Requested by the Observable abstract class in order to 
	// correctly dispatch the message
	void operator()(const ds::Notification &notification) const override {
		auto obj = notification.get_object();
		if (obj != nullptr) {
			// we **MUST** know it's type, at least for now..
			std::cout << notification.get_tag()
			          << ": "
			          << (char *)obj
			          << std::endl;
		}
	}
};

int main(int argc, char **argv) {
	auto t = std::string {"Test Object"};
	auto test = std::make_shared<Test> ();
	auto nc = ds::NotificationCenter::instance();
	nc->AddObserver("NotificationTag", test);
	nc->PostNotification("NotificationTag", t.c_str());
	return 0;
}
```