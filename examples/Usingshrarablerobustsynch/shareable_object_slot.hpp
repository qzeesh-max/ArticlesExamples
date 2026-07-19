#ifndef __SHAREABLE_OBJECT_SLOT__
#define __SHAREABLE_OBJECT_SLOT__


#ifdef _WIN32

#include <objbase.h>

#else

#include <uuid/uuid.h>


#endif

#include <mutex>
#include <cstdint>
#include <map>
#include <memory.h>
#include <iostream>
#include <iomanip>




template <typename T> class shareable_object_slot
{
public:

	class UUID
	{
	protected:
#ifdef _WIN32
		GUID   uniqueIdentifier;
#else
		uuid_t uniqueIdentifier;
#endif

	public:
#ifdef _WIN32
		operator GUID*()
		{
			return &uniqueIdentifier;
		}
#else
		operator uuid_t&()
		{
			return uniqueIdentifier;
		}
#endif

		bool operator<(const UUID& other) const
		{
			return memcmp(&uniqueIdentifier, &other.uniqueIdentifier, sizeof(uniqueIdentifier)) < 0;
		}

		std::uint32_t operator[](int index)
		{
			return reinterpret_cast<std::uint32_t*>(&uniqueIdentifier)[index];
		}
	};

protected:
	typedef std::map<UUID, T> SharedMap_t;


	static std::mutex& syncRoot()
	{
		static std::mutex _syncRoot;

		return _syncRoot;
	}

	static SharedMap_t& sharedMap()
	{
		static SharedMap_t _sharedMap;

		return _sharedMap;
	}

	UUID identifier;
	
public:
	shareable_object_slot()
	{
#ifdef _WIN32
		CoCreateGuid(identifier);
#else
		uuid_generate(identifier);
#endif
	}

	shareable_object_slot(const T& data) : shareable_object_slot()
	{
		*this = data;
	}

	~shareable_object_slot()
	{
		std::unique_lock<std::mutex> lock(syncRoot());

		sharedMap().erase(identifier);
	}	  	

	const UUID& get_uid() const
	{
		return identifier;
	}

	shareable_object_slot& operator=(const T& data)
	{
		std::unique_lock<std::mutex> lock(syncRoot());

		sharedMap()[identifier] = data;

		return *this;
	}


	bool operator==(const T& data)
	{
		std::unique_lock<std::mutex> lock(syncRoot());

		return sharedMap()[identifier] == data;
	}

	bool operator!=(const T& data)
	{
		std::unique_lock<std::mutex> lock(syncRoot());

		return sharedMap()[identifier] != data;
	}

	T& operator*()
	{
		std::unique_lock<std::mutex> lock(syncRoot());

		return sharedMap()[identifier];
	}


	T* operator->()
	{
		std::unique_lock<std::mutex> lock(syncRoot());

		return &sharedMap()[identifier];
	}

	
	
};

#endif
