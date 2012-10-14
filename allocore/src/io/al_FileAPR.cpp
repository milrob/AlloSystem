#include <cstring>
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Printing.hpp"

// TODO:
// At the moment, we only use APR for getting modification times of files
// and for scanning directories. It would be better to have native solutions
// for these two tasks...

#include "../private/al_ImplAPR.h"
#ifdef AL_LINUX
	#include "apr-1.0/apr_file_info.h"
#else
	#include "apr-1/apr_file_info.h"
#endif

namespace al{

struct File::Impl : public ImplAPR {
public:
	mutable apr_finfo_t finfo;
	
	Impl() : ImplAPR() {}
	virtual ~Impl() {}

	bool getInfo(const char * path, apr_int32_t wanted) const {
		return (APR_SUCCESS == check_apr(apr_stat(&finfo, path, wanted, mPool)));
	}
};

class Path : public ImplAPR {
public:
	apr_dir_t * dir;
	apr_finfo_t dirent;
	std::string dirname;
	
	Path(const std::string& dirname) : ImplAPR(), dir(NULL), dirname(dirname) {
		if (APR_SUCCESS != check_apr(apr_dir_open(&dir, dirname.c_str(), mPool))) {
			//printf("dir %p\n", dir);
			dir=NULL;
		}
	}
	
	virtual ~Path() {
		if (dir != NULL) check_apr(apr_dir_close(dir));
	}
	
	bool find(const std::string& name, FilePath& result, bool recursive=true) {
		bool found = false;
		if (dir && APR_SUCCESS == check_apr(apr_dir_open(&dir, dirname.c_str(), mPool))) {
			// iterate over directory:
			while ((!found) && APR_SUCCESS == (apr_dir_read(&dirent, APR_FINFO_TYPE|APR_FINFO_NAME, dir))) {
				//printf("test %s %s\n", dirname.c_str(), dirent.name);
				if (dirent.filetype == APR_REG && dirent.name && std::string(dirent.name) == name) {
					result.file(dirent.name);
					result.path(dirname);
					found = true;
					break;
				} else if (recursive && dirent.filetype == APR_DIR && dirent.name && dirent.name[0] != '.') {
					Path path(dirname + dirent.name + AL_FILE_DELIMITER);
					found = path.find(name, result, true);
				}
			}
		} else {
			AL_WARN("couldn't open directory %s", dirname.c_str());
		}
		return found;
	}
};


File::File(const std::string& path, const std::string& mode, bool open_)
:	mImpl(new Impl()), 
	mPath(path), mMode(mode), mContent(0), mSizeBytes(0), mFP(0)
{	if(open_) open(); }

File::File(const FilePath& path, const std::string& mode, bool open_)
:	mImpl(new Impl()), 
	mPath(path.filepath()), mMode(mode), mContent(0), mSizeBytes(0), mFP(0)
{	if(open_) open(); }

File::~File(){
	dtor();
	delete mImpl;
}

al_sec File :: modified() const {
	if (mImpl->getInfo(path().c_str(), APR_FINFO_MTIME)) {
		return 1.0e-6 * al_sec(mImpl->finfo.mtime);
	}
	return 0;
}

al_sec File :: accessed() const {
	if (mImpl->getInfo(path().c_str(), APR_FINFO_ATIME)) {
		return 1.0e-6 * al_sec(mImpl->finfo.atime);
	}
	return 0;
}

al_sec File :: created() const {
	if (mImpl->getInfo(path().c_str(), APR_FINFO_CTIME)) {
		return 1.0e-6 * al_sec(mImpl->finfo.ctime);
	}
	return 0;
}

size_t File :: sizeFile() const {
	if (mImpl->getInfo(path().c_str(), APR_FINFO_SIZE)) {
		return mImpl->finfo.size;
	}
	return 0;
}

size_t File :: storage() const {
	if (mImpl->getInfo(path().c_str(), APR_FINFO_CSIZE)) {
		return mImpl->finfo.csize;
	}
	return 0;
}


// This is the one sour egg that needs APR impl for directory scanning...
FilePath SearchPaths::find(const std::string& name) {
	FilePath result;
	bool found = false;
	std::list<SearchPaths::searchpath>::iterator iter = mSearchPaths.begin();
	while ((!found) && iter != mSearchPaths.end()) {
		Path path(iter->first.c_str());
		found = path.find(name, result, iter->second);
		iter++;
	}
	return result;
}

} // al::

