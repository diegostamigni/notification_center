//
// The MIT License (MIT)
//
// Copyright (c) 2015 Diego Stamigni
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <vector>
#include <utility>
#include <string>
#include <memory>
#include <algorithm>
#include <mutex>

namespace ds {
class Notification {
  public:
	Notification() : obj_(nullptr), tag_("") {}
	explicit Notification(const std::string &tag, const void *blob)
		: tag_(tag) {
		set_object(blob);
	}

	void *get_object() const { return obj_; }
	void set_object(const void *blob) {
		obj_ = (void *) blob;
	}

	std::string get_tag() const { return tag_; }
	void set_tag(const std::string &tag) { tag_ = tag; }

  private:
	void *obj_;
	std::string tag_;
};

class Observable {
  public:
	virtual void operator()(const ds::Notification &n) const = 0;
};

class NotificationCenter {
  public:
	NotificationCenter () {};
	NotificationCenter (NotificationCenter &&) = delete;
	NotificationCenter (const NotificationCenter &) = delete;
	NotificationCenter &operator=(const NotificationCenter &) = delete;
	NotificationCenter &operator=(NotificationCenter &&) = delete;

	// Singleton strategy with the new C++ meaning of `static'
	// Probably we don't need any unique/shared_ptr though...
	template<typename... Args>
	static NotificationCenter *instance(Args&&... args) {
		static NotificationCenter *i =
		    new NotificationCenter(std::forward<Args>(args)...);
		return i;
	}

	// Append a new observer with a certain key
	// --
	// Example: AddObserver("MyNotification", this)
	void AddObserver(const std::string &id,
	                 const std::shared_ptr<Observable> &f) {
		std::lock_guard<std::mutex> guard(lock_);
		observers_.emplace_back(id, f);
	}

	// Iterate through all the saved observers in order to
	// dispatch the message to them by calling it's delegates
	// --
	// Example: PostNotification("MyNotification", myObj)
	template <class T>
	void PostNotification(const std::string &id, T&& object = nullptr) {
		std::lock_guard<std::mutex> guard(lock_);
		std::for_each(observers_.begin(), observers_.end(), [&](auto & obj) {
			if (obj.first == id) {
				if (obj.second != nullptr) {
					Notification notif(id, object);
					(*obj.second)(std::move(notif));
				}
			}
		});
	}

	// Removes any entry founded with the gived ID.
	// Example: RemoveObservers("MyNotification")
	void RemoveObservers(const std::string &id) {
		if (observers_.size() < 1) return;
		std::lock_guard<std::mutex> guard(lock_);
		std::vector<std::size_t> tmp;
		tmp.reserve(observers_.size());
		for (auto obj = observers_.begin();
		        obj != observers_.end(); obj++) {
			if (obj->first == id) {
				// adding indexes
				auto index = observers_.end() - obj;
				tmp.push_back(index);
			}
		}
		for (auto &obj : tmp) {
			// removing by index
			auto index = observers_.end() - obj;
			observers_.erase(index);
		}
	}

  private:
	// Contains refs to observers.
	// An observer is a struct of `caller_id' and an its `observer'.
	// --
	// Couldn't be a map though because hashes don't provide us
	// the correct data structure. What we need to manage is:
	//   - 	multiple keys associated with different observers
	//	 - 	operate in order to manipulate the data structure
	//		for the related operations
	using observer_ = std::pair <std::string, std::shared_ptr<Observable>>;
	std::vector<observer_> observers_;
	std::mutex lock_;
};
} // namespace ds