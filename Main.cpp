#include <iostream>
#include <vector>
#include <atomic>
#include <mutex>

#include <time.h>
#include "Timer.h"
#include "TaskQueue.h"

/*
Write a parallel version of quicksort and linear search.  Run with 2-8 threads in the thread pool.
Use std::sort and std::find as your serial algorithm to compare with for speedup.
Search and sort vectors of 100 to 1,000,000 elements (use a log scale).  Include a report of timing, speedup, and efficiency.
*/

double getAverage(std::vector<double> times)
{
	int size = times.size();
	double total = 0;
	for (int i = 0; i < times.size(); i++)
	{
		total += times[i];
	}
	double average = total / size;
	return average;
}

double getStdDev(double average, std::vector<double> times)
{
	double size = times.size();
	double sum = 0;
	for (double i = 0; i < times.size(); i++)
	{
		sum += ((times[i] - average) * (times[i] - average));
	}
	sum = sqrt(sum / size);
	return sum;
}

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

bool linearSearch(std::vector<int>elements, int search_value, int start, int end, TaskQueue &queue, std::atomic<bool> &element_found)
{
	int increment = start;
	while (increment < end && element_found == false)
	{
		if (elements[increment] == search_value)
		{
			element_found = true;
			return true;
		}
		else increment++;
	}
	return false;
}

bool runlinearSearch(TaskQueue &queue, int search_val, std::vector<int> randomNumbers, int j)
{
	std::atomic<bool> element_found = false;
	int start;
	int end = 0;
	for (int i = 0; i < j; i++)
	{
		start = end;
		end = (randomNumbers.size() * (i + 1)) / j;
		queue.add_task([=, &queue, &element_found]() {linearSearch(randomNumbers, search_val, start, end, queue, element_found); });
		end++;
	}

	//if (element_found) std::cout << "found: " << search_val << std::endl;
	return element_found;
}

std::vector<std::vector<int>> getRandomVector(int element_count)
{
	srand(time(NULL));
	std::vector<std::vector<int>> number_vectors;
	for (int j = 0; j < 6; j++)
	{
		std::vector<int> numbers;
		for (int i = 0; i < element_count; i++)
		{
			numbers.push_back(rand() % 200 + 1);
		}
		number_vectors.push_back(numbers);

	}
	return  number_vectors;
}

void print_vector(std::vector<int> numbers)
{
	std::cout << std::endl;
	for (int i = 0; i < numbers.size(); i++)
	{
		std::cout << numbers[i] << std::endl;
	}
	std::cout << std::endl;
}

int main()
{
	srand(time(NULL));

	std::vector<std::vector<int>> standard_test_numbers = getRandomVector(200);

	std::vector<double> standard_sort_times;
	std::vector<double> standard_search_times;

	//STANDARD quick sort
	for (int i = 0; i < 5; i++)
	{
		double time = functionTimer([&]() {std::sort(standard_test_numbers[i].begin(), standard_test_numbers[i].end()); });
		standard_sort_times.push_back(time);
	}

	////STANDARD linear search
	for (int i = 0; i < 5; i++)
	{
		int find_value = rand() % 200 + 1;
		double time = functionTimer([=]() {std::find(standard_test_numbers[i].begin(), standard_test_numbers[i].end(), find_value); });
		standard_search_times.push_back(time);
	}

	double standard_sort = getAverage(standard_sort_times);
	double standard_dev_sort = getStdDev(standard_sort, standard_sort_times);
	std::cout << "STANDARD SORT" << std::endl;
	std::cout << "Average SEARCH time: " << standard_sort << std::endl;
	std::cout << "Standard Deviation: " << standard_dev_sort << std::endl;
	std::cout << std::endl;

	double standard_search = getAverage(standard_search_times);
	double standard_dev_search = getStdDev(standard_search, standard_search_times);
	std::cout << "STANDARD SEARCH" << std::endl;
	std::cout << "Average SORT time: " << standard_search << std::endl;
	std::cout << "Standard Deviation: " << standard_dev_search << std::endl;
	std::cout << std::endl;


	for (int j = 2; j <= 8; j++)
	{
		std::vector<double> sort_times;
		std::vector<double> search_times;

		std::vector<std::vector<int>> randomNumbers = getRandomVector(200);
		TaskQueue queue(j);

		//print out results of sort one time
		if (j == 2)
		{
			quickSort(randomNumbers[5], 0, randomNumbers[5].size() - 1, queue);
			print_vector(randomNumbers[5]);
		}

		//quick sort
		for (int i = 0; i < 5; i++)
		{
			double time = functionTimer([=, &queue, &randomNumbers]() {quickSort(randomNumbers[i], 0, randomNumbers[i].size() - 1, queue); });
			sort_times.push_back(time);
		}

		//linear search
		for (int i = 0; i < 5; i++)
		{
			int find_value = rand() % 200 + 1;
			double time = functionTimer([=, &queue, &randomNumbers]() {if (runlinearSearch(queue, find_value, randomNumbers[i], j)) std::cout << "found: " << find_value << std::endl; });
			search_times.push_back(time);
		}

		double sort_average = getAverage(sort_times);
		double sort_std_dev = getStdDev(sort_average, sort_times);
		std::cout << "THREADED WITH " << j << " THREADS" << std::endl;
		std::cout << "Average SEARCH time: " << sort_average << std::endl;
		std::cout << "Standard Deviation: " << sort_std_dev << std::endl;
		std::cout << std::endl;

		double search_average = getAverage(search_times);
		double search_std_dev = getStdDev(search_average, search_times);
		std::cout << "THREADED WITH " << j << " THREADS" << std::endl;
		std::cout << "Average SORT time: " << sort_average << std::endl;
		std::cout << "Standard Deviation: " << sort_std_dev << std::endl;
		std::cout << std::endl;
	}

	return 0;
}
