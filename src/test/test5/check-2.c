int lzma_check_is_supported(unsigned int type)
{
	if (type > 1)
		return 0;

	static const unsigned int available_checks[2] = { 1, 0 };
	return available_checks[type];
}
