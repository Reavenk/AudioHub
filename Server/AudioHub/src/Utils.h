#include <assert.h>
#include <mutex>
#include <memory>

#define ASSERT_NOTIN(container, key) \
	assert(std::find(container.begin(), container.end(), key) == container.end());

#define ASSERT_IN(container, key) \
	assert(std::find(container.begin(), container.end(), key) != container.end());

#define ASSERT_INFIND(container, key) \
	assert(container.find(key) != container.end());

#define ASSERT_NOTINFIND(container, key) \
	assert(container.find(key) == container.end());

#define LOCKGUARD(mut, name) \
	std::lock_guard<std::mutex> guard##name(mut);

// NOTE: The typing below is getting out of hand. Probably a cleaner way without so many <<<>>>'s
#define OPTIONAL_LOCKGUARD(mut, name, dolock) \
	std::shared_ptr<std::lock_guard<std::mutex>> optguard##name(nullptr); \
	if(dolock) \
		optguard##name = std::shared_ptr<std::lock_guard<std::mutex>>(new std::lock_guard<std::mutex>(mut));
