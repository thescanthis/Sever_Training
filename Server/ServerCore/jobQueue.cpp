#include "pch.h"
#include "JobQueue.h"

void JobQueue::Push(JobRef&& job)
{
	//Add를 할때 이전 값을 가지고옴
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job);

	//첫번째 Job을 넣은 쓰레드가 실행까지 담당
	if (prevCount == 0)
	{
		Execute();
	}
}

void JobQueue::Execute()
{
	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(OUT jobs);
		
		const int32 jobCount = static_cast<int32>(jobs.size());

		for (int32 i = 0; i < jobCount; i++)
			jobs[i]->Execute();

		//남은 일감이 0개라면 종료
		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			return;
		}
	}
}
