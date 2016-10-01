#include <iostream>
#include <vector>
#include <time.h>
#include "Timer.h"
#include "TaskQueue.h"

/*
Write a parallel version of quicksort and linear search.  Run with 2-8 threads in the thread pool.
Use std::sort and std::find as your serial algorithm to compare with for speedup.
Search and sort vectors of 100 to 1,000,000 elements (use a log scale).  Include a report of timing, speedup, and efficiency.
*/

int get_median(std::vector<int>elements, int first, int second, int third)
{
	if (elements[first] < elements[second] && elements[first] < elements[third])
	{
		if (elements[second] < elements[third]) return second;
		else return third;
	}
	else if (elements[first] > elements[second] && elements[first] > elements[third])
	{
		if (elements[second] > elements[third]) return second;
		else return third;
	}
	else return first;
}

void quickSort(std::vector<int> &elements, int start, int end)
{
	while (start < end)
	{
		int middle = (start + end) / 2;
		int pivot = get_median(elements, start, middle, end);
		int greater_than_index = start;
		//move pivot out of the way to the end
		std::swap(elements[pivot], elements[end]);
		for (int i = start; i < end; i++)
		{
			if (elements[i] < elements[end])
			{
				//swap element with first element in greater_than range and scoot greater_than range over one
				std::swap(elements[i], elements[greater_than_index]);
				greater_than_index++;
			}
		}
		//swap pivot back into place
		std::swap(elements[end], elements[greater_than_index]);
		quickSort(elements, greater_than_index + 1, end);
		start = start;
		end = greater_than_index - 1;
	}
	return;
}

void quickSort(std::vector<int> &elements, int start, int end, TaskQueue &queue)
{
	while (start < end)
	{
		int middle = (start + end) / 2;
		int pivot = get_median(elements, start, middle, end);
		int greater_than_index = start;
		std::swap(elements[pivot], elements[end]);
		for (int i = start; i < end; i++)
		{
			if (elements[i] < elements[end])
			{
				std::swap(elements[i], elements[greater_than_index]);
				greater_than_index++;
			}
		}
		std::swap(elements[end], elements[greater_than_index]);
		queue.add_task([=, &elements, &queue]() {quickSort(elements, greater_than_index + 1, end, queue); });
		start = start;
		end = greater_than_index - 1;
	}
	return;
}

bool linearSearch(std::vector<int>elements, int search_value, int start, int end, TaskQueue &q)
{
	int increment = start;
	while (increment < end)
	{
		if (elements[increment] == search_value)
		{
			//std::cout << "TRUE" << std::endl;
			return true;
		}
		else increment++;
	}
	//std::cout << "FALSE" << std::endl;
	return false;
}

std::vector<int> getRandomVector(int element_count)
{
	srand(time(NULL));
	std::vector<int> numbers;
	for (int i = 0; i < element_count; i++)
	{
		numbers.push_back(rand() % 20 + 1);
	}
	return  numbers;
}

void print_vector(std::vector<int> numbers)
{
	for (int i = 0; i < numbers.size(); i++)
	{
		std::cout << numbers[i] << std::endl;
	}
}

int main()
{
	srand(time(NULL));

	for (int j = 2; j <= 8; j++)
	{
		std::vector<int> randomNumbers = getRandomVector(20);
		TaskQueue queue(j);
		for (int i = 0; i < 5; i++)
		{
			//select a partition of the vector
			quickSort(randomNumbers, 0, randomNumbers.size() - 1, queue);
			//print_vector(randomNumbers);
			std::cout << i;
		}
		std::cout << std::endl;


		for (int i = 0; i < 5; i++)
		{
			int find_value = rand() % 20 + 1;
			//select a partion of the vector
			int start;
			int end = 0;
			for (int i = 0; i < j; i++)
			{
				start = end;
				end = (randomNumbers.size() * (i + 1)) / j;
				queue.add_task([=, &queue]() {linearSearch(randomNumbers, find_value, start, end, queue); });
				end++;
			}
			std::cout << i;
		}
		std::cout << std::endl;


	}

	return 0;
}