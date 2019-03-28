
#ifndef __PAGEMAP_H
#define __PAGEMAP_H

#include <atomic>

#include "pm_config.hpp"
#include "pfence_util.h"
#include "SizeClass.hpp"
#include "RegionManager.hpp"
// assuming x86-64, for now
// which uses 48 bits for addressing (e.g high 16 bits ignored)
// can ignore the bottom 12 bits (lg of page)
// insignificant high bits
#define PM_NHS 14
// insignificant low bits. page is 4K
#define PM_NLS 12
// significant middle bits
#define PM_SB (64 - PM_NHS - PM_NLS)
// to get the key from a address
// 1. shift to remove insignificant low bits
// 2. apply mask of middle significant bits
#define PM_KEY_SHIFT PM_NLS
#define PM_KEY_MASK ((1ULL << PM_SB) - 1)

struct Descriptor;
// associates metadata to each allocator page
// implemented with a static array, but can also be implemented
//  with a multi-level radix tree

#define SC_MASK ((1ULL << 6) - 1)

// contains metadata per page
// *has* to be the size of a single word
struct PageInfo
{
private:
	// descriptor
	Descriptor* _desc;
	// size class
	// stealing bits from desc to store size class
	// desc is aligned to at least 64 bytes, so 6 bits to steal
	// which is the same as bits for sizeclass idx

public:
	void set(Descriptor* desc, size_t sc_idx);
	Descriptor* get_desc() const;
	size_t get_sc_idx() const;
};

#define PM_SZ ((1ULL << PM_SB) * sizeof(PageInfo))

static_assert(sizeof(PageInfo) == sizeof(uint64_t), "Invalid PageInfo size");

// lock free page map
class PageMap
{
public:
	PageMap();
	~PageMap();
	PageInfo get_page_info(char* ptr);
	void set_page_info(char* ptr, PageInfo info);

private:
	size_t addr_to_key(char* ptr) const;

	// array based impl
	std::atomic<PageInfo>* _pagemap = { nullptr };
	RegionManager* mgr = nullptr;//initialized when ArrayStack constructs
};

#endif // __PAGEMAP_H

