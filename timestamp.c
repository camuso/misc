long get_timestamp()
{
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	return (ts.tv_sec << 32) + ts.tv_nsec;
}

