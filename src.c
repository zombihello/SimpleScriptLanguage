void test( int a, string z )
{
	int q;
	q = a;
	print( z, "A =", q );
	
	print( "Enter string: " );
	scan( z );
}

void main()
{
	print( "~~Execute script~~" );

	int _a;
	int tmp_a;
	_a = 2;
	
	tmp_a = _a + 2;
	print( "_a+2", tmp_a, " source _a=", _a );

	tmp_a = _a - 2;
	print( "_a-2", tmp_a, " source _a=", _a );

	tmp_a = _a * 2;
	print( "_a*2", tmp_a, " source _a=", _a );

	tmp_a = _a / 2;
	print( "_a/2", tmp_a, " source _a=", _a );

	int		isRequestExit;
	isRequestExit = 0;

	string userText;
	userText = "test";

	while ( isRequestExit != 1 )
	{
		int		a;
		a = 0;
		
		print( "Enter a: " );
		scan( a );

		a = a + 1;
		print( "Result a+1", a );

		if ( a >= 1 )
		{
			print( "True branch 'a == 1'" );
			test( a, userText );
		}
		else
		{
			if ( userText != "test" )
			{
				print( "False branch 'a == 1'" );
				test( 2, "hello" );
			}
		}
		
		print( "" );
		print( "Is need exit? (0 - No, 1 - Yes)" );
		scan( isRequestExit );
	}
}