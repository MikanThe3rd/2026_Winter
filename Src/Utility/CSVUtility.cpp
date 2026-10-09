#include <fstream>
#include <sstream>
#include "CSVUtility.h"

namespace
{
	// ‘OŒã‚Ì‹ó”’‚ğœ‹
	std::string Trim(const std::string& s)
	{
		const char* ws = " \t\r\n";
		size_t b = s.find_first_not_of(ws);
		if (b == std::string::npos)
		{
			return "";
		}
		size_t e = s.find_last_not_of(ws);
		return s.substr(b, e - b + 1);
	}
}

namespace CsvUtility
{
	bool Load(const std::string& path,
		std::vector<std::vector<std::string>>& rows)
	{
		std::ifstream ifs(path);
		if (!ifs)
		{
			return false;
		}

		std::string line;
		while (std::getline(ifs, line))
		{
			line = Trim(line);
			if (line.empty() || line[0] == '#')
			{
				continue;
			}

			std::vector<std::string> cols;
			std::stringstream ss(line);
			std::string cell;
			while (std::getline(ss, cell, ','))
			{
				cols.push_back(Trim(cell));
			}
			rows.push_back(std::move(cols));
		}
		return true;
	}

	float ToFloat(const std::vector<std::string>& cols, size_t idx, float def)
	{
		if (idx >= cols.size() || cols[idx].empty())
		{
			return def;
		}
		try
		{
			return std::stof(cols[idx]);
		}
		catch (const std::exception&)
		{
			return def;
		}
	}

	int ToInt(const std::vector<std::string>& cols, size_t idx, int def)
	{
		if (idx >= cols.size() || cols[idx].empty())
		{
			return def;
		}
		try
		{
			return std::stoi(cols[idx]);
		}
		catch (const std::exception&)
		{
			return def;
		}
	}
}