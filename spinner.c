
void lookup::show_spinner(int& count)
{
	if (count % 0x10000)
		cout << "|\r";
	if (count % 0x14000)
		cout << "\\\r";
	if (count % 0x18000)
		cout << "-\r";
	if (count % 0x1c000)
		cout << "/\r";

	cout.flush();
	++count;
}

