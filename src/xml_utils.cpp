#include "xml_utils.hpp"

std::string_view extract_xml_value(std::string_view xml_attr) {
	size_t eq_index = xml_attr.find('=');
	return xml_attr.substr(eq_index + 2, xml_attr.size() - (eq_index + 2) - 1);
}
