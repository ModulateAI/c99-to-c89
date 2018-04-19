lzma_check_is_supported(lzma_check type)
{
	if ((unsigned int)(type) > 15)
		return 0;

	static const lzma_bool available_checks[15 + 1] = {
		1,   
		1,
		0,  
		0,  
		1,
		0,  
		0,  
		0,  
		0,  
		0,  
		1,
		0,  
		0,  
		0,  
		0,  
		0,  
	};
	return available_checks[(unsigned int)(type)];
}
