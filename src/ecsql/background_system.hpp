#include <functional>
#include <string>
#include <string_view>

namespace ecsql {

class BackgroundSystem {
public:
	BackgroundSystem(std::string_view name, std::function<void()> implementation, bool join_before_new_frame = true);

	void operator()();

	const std::string& get_name() const;
	bool should_join_before_new_frame() const;

private:
	std::string name;
	std::function<void()> implementation;
	bool join_before_new_frame;
};

}
