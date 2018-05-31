/* $LastChangedDate: 2017-09-10 14:53:32 +0200 (Sun, 10 Sep 2017) $ */
/*
 Cache.h : Cache manager header file

 (c) 2007-2017 Michele Martone

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/
#ifndef FIM_CACHE_H
#define FIM_CACHE_H
#include "fim.h"

#ifdef HAVE_SYS_TIME_H
#if FIM_USE_CXX11
#include <cstdint>
#else /* FIM_USE_CXX11 */
#include <stdint.h>
#endif /* FIM_USE_CXX11 */
#include <unistd.h>
typedef uint64_t fim_time_t;
#else /* HAVE_SYS_TIME_H */
typedef time_t fim_time_t;
#endif /* HAVE_SYS_TIME_H */
#define FIM_CR_BS 0 /* base */
#define FIM_CR_CN 1 /* not detailed */
#define FIM_CR_CD 2 /* detailed */

namespace fim
{
class Cache FIM_FINAL
#ifdef FIM_NAMESPACES
	:private Namespace
#else
#endif /* FIM_NAMESPACES */
{
#if FIM_USE_CXX11
	using cachels_t =  std::map<cache_key_t,fim::ImagePtr >;	//key -> image
	using lru_t = std::map<cache_key_t,fim_time_t >;	//image -> time
	using ccachels_t = std::map<cache_key_t,int >;	//key -> counter
	using vcachels_t = std::map<cache_key_t,ViewportState >;	//key -> viewport state
	//using cloned_cachels_t = std::map<cache_key_t,std::vector<fim::ImagePtr> >;	//key -> clones
	//using cuc_t = std::map<fim::ImagePtr,int >;	//image -> clones usage counter
#else /* FIM_USE_CXX11 */
	typedef std::map<cache_key_t,fim::ImagePtr >  cachels_t;	//key -> image
	typedef std::map<cache_key_t,fim_time_t > 	   lru_t;	//image -> time
	typedef std::map<cache_key_t,int >        ccachels_t;	//key->counter
	typedef std::map<cache_key_t,ViewportState >        vcachels_t;	//key -> viewport state
	//typedef std::map<cache_key_t,std::vector<fim::ImagePtr> > cloned_cachels_t;	//key -> clones
	//typedef std::map<fim::ImagePtr,int >  	   cuc_t;	//image -> clones usage counter
#endif /* FIM_USE_CXX11 */

	cachels_t 	imageCache_;
	lru_t		lru_;
	ccachels_t	usageCounter_;
	vcachels_t	viewportInfo_;
	//cloned_cachels_t cloneCache_;
	//cuc_t		cloneUsageCounter_;
	std::set< fim::ImagePtr > clone_pool_;
	time_t time0_;

	bool need_free(void)const;
	int lru_touch(cache_key_t key);

	public:
	bool is_in_cache(cache_key_t key)const;
	private:
	bool is_in_clone_cache(fim::ImagePtr oi)const;
	fim_time_t last_used(cache_key_t key)const;
	bool cacheNewImage( fim::ImagePtr ni );
	ImagePtr loadNewImage(cache_key_t key, fim_page_t page, fim_bool_t delnc);
	ImagePtr getCachedImage(cache_key_t key);
	int erase(fim::ImagePtr oi);
	int erase_clone(fim::ImagePtr oi);
	ImagePtr get_lru( bool unused = false )const;
	int free_some_lru(void);
	int used_image(cache_key_t key)const;
	fim_time_t get_reltime(void)const;
	public:
	Cache(void);
#if FIM_USE_CXX11
	public:
	/* a deleted member function (e.g. not even a be'friend'ed class can call it) */
	Cache& operator= (const Cache&rhs) = delete;
#else /* FIM_USE_CXX11 */
	private:
	/* a disabled member function (because of private: keyword) */
	Cache& operator= (const Cache&rhs) { return *this; }
#endif /* FIM_USE_CXX11 */
	public:
	bool freeCachedImage(ImagePtr image, const ViewportState *vsp, bool force);
	ImagePtr useCachedImage(cache_key_t key, ViewportState *vsp, fim_page_t page = 0);
	ImagePtr setAndCacheStdinCachedImage(ImagePtr image);
	int prefetch(cache_key_t key);
	void touch(cache_key_t key);
	fim::string getReport(int type = FIM_CR_CD )const;
	virtual size_t byte_size(void)const;
	size_t img_byte_size(void)const;
	int cached_elements(void)const;
	void desc_update(void);
	~Cache(void);
};
}

#if FIM_WANT_BACKGROUND_LOAD
#include <thread>
#include <chrono>
#include <mutex>

namespace fim
{

typedef fim::string fid_t;

class PACA FIM_FINAL	/* Parallel Cache */
{
	public:
#if FIM_WANT_BACKGROUND_LOAD
	std::thread t;
#endif /* FIM_WANT_BACKGROUND_LOAD */
	Cache& cache_;
	static FIM_CONSTEXPR int dpc = 0; /* debug parallel cache */

	void operator () (const fid_t& rid)
	{
		if(dpc) std::cout << __FUNCTION__ << ": "<< rid << " ...\n";
		if(dpc) std::cout << (&cache_) << " : " << cache_.getReport() << "\n";

		if(dpc) if( ! cache_.is_in_cache(cache_key_t(rid,FIM_E_FILE)))
			std::cout << __FUNCTION__ << ": "<< rid << " ... sleeping\n",
			std::this_thread::sleep_for(std::chrono::milliseconds(4000));
		cache_.prefetch(cache_key_t(rid,FIM_E_FILE));
		if(dpc) std::cout << __FUNCTION__ << ": "<< rid << " ... done\n";
	}

	explicit PACA(const PACA & paca) : cache_(paca.cache_)
       	{
		if(dpc) std::cout << __FUNCTION__ << "\n";
	}
	explicit PACA(Cache& cache):cache_(cache) { }

	ImagePtr useCachedImage(cache_key_t key, ViewportState *vsp, fim_page_t page=0)
	{
		ImagePtr image = FIM_NULL;
		if(dpc) std::cout << __FUNCTION__ << ": " << key.first << "\n";
		if( t.joinable() )
		{
			if(dpc) std::cout << __FUNCTION__ << ": " << key.first << " join()\n";
	       		t.join();
			if(dpc) std::cout << __FUNCTION__ << ": " << key.first << " joined()\n";
		}
		image = cache_.useCachedImage(key, vsp, page);
		if(dpc) std::cout << __FUNCTION__ << ": " << key.first << " done\n";
		if(dpc) std::cout << (&cache_) << " : " << cache_.getReport() << "\n";
		return image;
	}

	void asyncPrefetch(const fid_t& rid)
	{
		if(dpc) std::cout << __FUNCTION__ << ": " << rid << "\n";
		if( t.joinable() )
		{
			//if(dpc) std::cout << __FUNCTION__ << ": " << rid << " return\n"; return;
			if(dpc) std::cout << __FUNCTION__ << ": " << rid << " join()\n";
	       		t.join();
		}
		t = std::thread(*this,rid); // operator ()
	}
	~PACA(void)
	{
		if( t.joinable() )
	       		t.join();
	}
}; /* PACA */
} /* namespace */
#endif /* FIM_WANT_BACKGROUND_LOAD */
#endif /* FIM_CACHE_H */
