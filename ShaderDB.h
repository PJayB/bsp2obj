#pragma once

#include <string>

class Shader
{
};

class ShaderDB
{
public:

	ShaderDB(void);
	~ShaderDB(void);

	bool Parse( const std::string& shaderSource );

private:


};

