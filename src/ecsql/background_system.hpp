#include <functional>
#include <string>
#include <string_view>

namespace ecsql {

class BackgroundSystem {
public:
	BackgroundSystem(std::string_view name, std::function<void()> implementation);

	void operator()();

	const std::string& get_name() const;

private:
	std::string name;
	std::function<void()> implementation;
};

}
