#ifndef ShaderProgram_h__
#define ShaderProgram_h__

#include "Prerequisite.h"
#include "GraphicCommon.h"

class ShaderProgram
{
public:
	ShaderProgram(void);
	~ShaderProgram(void);

public:
	virtual void ExecuteVS(const VS_Input& input, VS_Output* output) = 0;
	virtual void ExecutePS(const VS_Output& input) = 0;

private:
	std::vector<uint8_t> mConstantBuffer;
};

#endif // ShaderProgram_h__
