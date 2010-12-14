#include "allocore/graphics/al_GraphicsOpenGL.hpp"
#include "allocore/graphics/al_Shader.hpp"

#include <map>
#include <string>

using std::map;
using std::string;

namespace al{

GLenum gl_shader_type(Shader::Type v) {
	switch(v){
		case Shader::FRAGMENT:	return GL_FRAGMENT_SHADER;
		case Shader::VERTEX:	return GL_VERTEX_SHADER;
		default: return 0;
	}
}

const char * ShaderBase::log(){
	GLint lsize; get(GL_INFO_LOG_LENGTH, &lsize);
	if(0==lsize) return "\0";
	newLog(lsize);
	//glGetShaderInfoLog(id(), 4096, NULL, mLog);
	glGetInfoLogARB(handle(), 4096, NULL, mLog);
	return mLog;
}

/*
GLuint glCreateProgram (void);
GLuint glCreateShader (GLenum type);
void glDeleteShader (GLuint shader);
void glDeleteProgram (GLuint program);
void glDetachShader(GLuint program, GLuint shader);
*/

Shader::Shader(const std::string& source, Shader::Type type)
:	mSource(source), mType(type){}

Shader& Shader::compile(){ validate(); glCompileShader(id()); return *this; }

bool Shader::compiled() const {
	GLint v;
	//GLhandleARB h = (GLhandleARB)mID;
	glGetObjectParameterivARB(handle(), GL_COMPILE_STATUS, &v);
	return v;
}

void Shader::get(int pname, void * params) const { glGetShaderiv(id(), pname, (GLint *)params); }

void Shader::onCreate(){
	//mID = glCreateShader(gl_shader_type(mType));
	mHandle = glCreateShaderObjectARB(gl_shader_type(mType));
	mID = (long)handle();
	if(mSource[0]){
		sendSource(); compile();
	}
}

void Shader::onDestroy(){ 
	glDeleteObjectARB(handle());
	//glDeleteShader(id()); 
}

void Shader::sendSource(){
	const char * s = mSource.c_str();
	//glShaderSource(id(), 1, &s, NULL);
	glShaderSourceARB(handle(), 1, &s, NULL);
}

Shader& Shader::source(const std::string& v){
	mSource = v;
	if(created()){
		sendSource();
		compile();
	}
	return *this;
}

Shader& Shader::source(const std::string& src, Shader::Type type){
	mType=type; return source(src);
}
	




const ShaderProgram& ShaderProgram::attach(Shader& s) { 
	validate();
	if (!s.compiled()) s.compile();
	glAttachObjectARB(handle(), s.handle());
	//glAttachShader(id(), s.id()); 
	return *this; 
}
const ShaderProgram& ShaderProgram::detach(const Shader& s) const { 
	//glDetachShader(id(), s.id()); 
	glDetachObjectARB(handle(), s.handle());
	return *this; 
}
const ShaderProgram& ShaderProgram::link() const { 
	//glLinkProgram(id()); 
	glLinkProgramARB(handle());
	
	int isValid;
	//glValidateProgram(id());
	glValidateProgramARB(handle());
	glGetProgramiv(id(), GL_VALIDATE_STATUS, &isValid);
	if (!isValid) {
		GraphicsGL::gl_error("ShaderProgram::link");
	}
	return *this; 
}

void ShaderProgram::onCreate(){ 
	mHandle = glCreateProgramObjectARB();
	mID = (long)handle();
	//mID = glCreateProgram(); 
}
void ShaderProgram::onDestroy(){ 
	//glDeleteProgram(id()); 
	glDeleteObjectARB(handle()); 
}

const ShaderProgram& ShaderProgram::use() const { 
	//glUseProgram(id()); 
	glUseProgramObjectARB(handle());
	return *this; 
}
void ShaderProgram::begin() const { 
	use(); 
}
void ShaderProgram::end() const { 
	//glUseProgram(0); 
	glUseProgramObjectARB(0);
}
bool ShaderProgram::linked() const { 
	GLint v; 
	get(GL_LINK_STATUS, &v); 
	return v; 
}
// GLint v; glGetProgramiv(id(), GL_LINK_STATUS, &v); return v; }

const ShaderProgram& ShaderProgram::uniform(const char * name, int v0){
	//glUniform1i(uniformLocation(name), v0); 
	glUniform1iARB(uniformLocation(name), v0);
	return *this;
}

const ShaderProgram& ShaderProgram::uniform(const char * name, float v0){
	//glUniform1f(uniformLocation(name), v0); 
	glUniform1fARB(uniformLocation(name), v0);
	return *this;
}

const ShaderProgram& ShaderProgram::attribute(const char * name, float v0){
	glVertexAttrib1fARB(uniformLocation(name), v0);
	return *this;
}

int ShaderProgram::uniformLocation(const char * name) const { 
	GLint loc = glGetAttribLocationARB(handle(), name);
	return loc; 
}

int ShaderProgram::attributeLocation(const char * name) const { 
	GLint loc = glGetUniformLocationARB(handle(), name);
	return loc; // glGetUniformLocation(id(), name); 
}

void ShaderProgram::get(int pname, void * params) const { 
	glGetProgramiv(id(), pname, (GLint *)params); 
}

void ShaderProgram::listParams() const {
	GLuint program = id();
	GLint numActiveUniforms = 0;
	GLint numActiveAttributes = 0;

	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numActiveAttributes);

	for(int j=0; j < numActiveUniforms; j++)
	{
		GLsizei length;
		GLint size;
		GLenum gltype;
		char name[256];

		glGetActiveUniform(program,
							j,
							256,
							&length,
							&size,
							&gltype,
							name);

		// check for array names
		if(name[ strlen(name)-3 ] == '[' && name[ strlen(name)-1 ] == ']') {
			name[ strlen(name)-3 ] = '\0';
		}
		
		printf("uniform %d(%s): type %d size %d length %d\n",
			j, name, param_type_from_gltype(gltype), size, length);

//		//could already have a param if the user set some values before compiling
//		map<string, ShaderProgram *>::iterator it = mParameters.find(name);
//		if(it != mParameters.end()) {
//			ShaderProgram::Type type = param_type_from_gltype(gltype);
//			ShaderProgram &p = *(it->second);
//			p.set_active(true);
//			p.set_location(j);
//			p.set_type(type);
//			p.set_count(size);
//		}
		/*
		Only use params defined in shader file
		else
		{
			ShaderProgram *p = new ShaderProgram(name, j, type, size);
			mParameters[ name ] = p;
		}*/
	}

	for(int j=0; j < numActiveAttributes; j++) {
		GLsizei length;
		GLint size;
		GLenum gltype;
		char name[256];	// could query for max char length

		glGetActiveAttrib(program,
							j,
							256,
							&length,
							&size,
							&gltype,
							name);
							
		printf("attribute %d(%s): type %d size %d length %d\n",
			j, name, param_type_from_gltype(gltype), size, length);

		//map<string, ShaderAttribute *>::iterator it = mAttributes.find(name);
//		if(it != mAttributes.end()) {
//			// TODO: FIX THIS HACK
//			#if defined(MURO_LINUX_VERSION)
//			int loc = (j < 0) ? 1 : j+1;
//			#else
//			int loc = (j <= 0) ? 1 : j;
//			#endif
//			ShaderProgram::Type type = param_type_from_gltype(gltype);
//			ShaderAttribute &a = *(it->second);
//			a.realize_location(loc);
//			a.set_type(type);
//			//a.setCount(size);
//		}
	}
}

ShaderProgram::Type ShaderProgram :: param_type_from_gltype(GLenum gltype) {
	ShaderProgram::Type type = ShaderProgram::NONE;

	switch(gltype) {
		case GL_FLOAT:			type = ShaderProgram::FLOAT;	break;
		case GL_FLOAT_VEC2:		type = ShaderProgram::VEC2;	break;
		case GL_FLOAT_VEC3:		type = ShaderProgram::VEC3;	break;
		case GL_FLOAT_VEC4:		type = ShaderProgram::VEC4;	break;

		case GL_INT:			type = ShaderProgram::INT;	break;
		case GL_INT_VEC2:		type = ShaderProgram::INT2;	break;
		case GL_INT_VEC3:		type = ShaderProgram::INT3;	break;
		case GL_INT_VEC4:		type = ShaderProgram::INT4;	break;

		case GL_BOOL:			type = ShaderProgram::BOOL;	break;
		case GL_BOOL_VEC2:		type = ShaderProgram::BOOL2;	break;
		case GL_BOOL_VEC3:		type = ShaderProgram::BOOL3;	break;
		case GL_BOOL_VEC4:		type = ShaderProgram::BOOL4;	break;

		case GL_FLOAT_MAT2:		type = ShaderProgram::MAT22;	break;
		case GL_FLOAT_MAT3:		type = ShaderProgram::MAT33;	break;
		case GL_FLOAT_MAT4:		type = ShaderProgram::MAT44;	break;

		case GL_SAMPLER_1D:		type = ShaderProgram::SAMPLER_1D;	break;
		case GL_SAMPLER_2D:		type = ShaderProgram::SAMPLER_2D;	break;
		case GL_SAMPLER_2D_RECT_ARB: type = ShaderProgram::SAMPLER_RECT; break;
		case GL_SAMPLER_3D:		type = ShaderProgram::SAMPLER_3D;	break;
		case GL_SAMPLER_CUBE:	type = ShaderProgram::SAMPLER_CUBE; break;
		case GL_SAMPLER_1D_SHADOW: type = ShaderProgram::SAMPLER_1D_SHADOW; break;
		case GL_SAMPLER_2D_SHADOW: type = ShaderProgram::SAMPLER_2D_SHADOW; break;
	}

	return type;
}

} // ::al


//Shader shader1;
//shader1.set(shaderBuf, GL_FRAGMENT_SHADER);
//
//ShaderProgram shaderProgram;
//
//shaderProgram.attach(shader1);
//shaderProgram.link().use();

/*

	char *vs = NULL,*fs = NULL,*fs2 = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	f2 = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("toon.vert");
	fs = textFileRead("toon.frag");
	fs2 = textFileRead("toon2.frag");

	const char * ff = fs;
	const char * ff2 = fs2;
	const char * vv = vs;

	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);
	glShaderSource(f2, 1, &ff2,NULL);

	free(vs);free(fs);

	glCompileShader(v);
	glCompileShader(f);
	glCompileShader(f2);

	p = glCreateProgram();
	glAttachShader(p,f);
	glAttachShader(p,f2);
	glAttachShader(p,v);

	glLinkProgram(p);
	glUseProgram(p);
*/