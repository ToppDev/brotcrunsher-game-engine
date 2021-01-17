#pragma once
#pragma once

#include "List.h"

namespace bbe
{
	template <typename T>
	class Stack
	{
	private:
		List<T> m_data;

	public:
		void push(const T &object)
		{
			m_data.pushBack(object);
		}

		T pop()
		{
			if (m_data.getLength() <= 0)
			{
				//TODO add further error handling
				__debugbreak();
			}

			T data = std::move(m_data.last());
			m_data.removeIndex(m_data.getLength() - 1);
			return data;
		}

		T peek()
		{
			if (m_data.getLength() <= 0)
			{
				//TODO add further error handling
				__debugbreak();
			}

			return m_data.last();
		}

		size_t dataLeft()
		{
			return m_data.getLength();
		}

		bool hasDataLeft()
		{
			return dataLeft() > 0;
		}
	};
}