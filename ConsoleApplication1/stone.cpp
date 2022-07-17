#define _CRT_SECURE_NO_WARNINGS
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <stack>
#include <cassert>
#include <unordered_map>
#include <memory>

std::size_t MemFastHash( const void* InData, std::size_t InLength, std::size_t InHash = 0 )
{
	unsigned char* data = ( unsigned char* ) InData;
	for ( std::size_t index = 0; index < InLength; ++index )
	{
		InHash = data[ index ] + ( InHash << 6 ) + ( InHash << 16 ) - InHash;
	}
	return InHash;
}

enum EScriptOperation
{
	Op_Nope,
	Op_Call,
	Op_NativeCall,
	Op_AllocateVar,
	Op_Assign,
	Op_Compare,
	Op_NotCompare,
	Op_More,
	Op_MoreThen,
	Op_Less,
	Op_LessThen,
	Op_JumpNotEqual,
	Op_JumpEqual,
	Op_Jump,
	Op_Add,
	Op_Substruct,
	Op_Multiply,
	Op_Divide,
};

enum EScriptVarType
{
	SVT_None,
	SVT_String,
	SVT_Int,
	SVT_Bool
};

enum EScriptVarFlag
{
	SVF_User,
	SVF_Const,
	SVF_Arg,
	SVF_Register
};

enum EScriptRegister
{
	SR_AX,
	SR_Num
};

class FScriptVar
{
public:
	FScriptVar()
		: ptr( nullptr )
	{
	}

	FScriptVar( const FScriptVar& InCopy )
	{
		switch ( InCopy.varType )
		{
		case SVT_String:    SetString( InCopy.GetString() );    break;
		case SVT_Int:       SetInt( InCopy.GetInt() );          break;
		case SVT_Bool:       SetBool( InCopy.GetBool() );          break;
		}
	}

	~FScriptVar()
	{
		Clear();
	}

	FScriptVar& operator=( const FScriptVar& InCopy )
	{
		switch ( InCopy.varType )
		{
		case SVT_String:    SetString( InCopy.GetString() );    break;
		case SVT_Int:       SetInt( InCopy.GetInt() );          break;
		case SVT_Bool:      SetBool( InCopy.GetBool() );          break;
		}
		return *this;
	}

	static std::shared_ptr<FScriptVar> Add( std::shared_ptr<FScriptVar> InLeft, std::shared_ptr<FScriptVar> InRight )
	{
		std::shared_ptr<FScriptVar>		result = std::make_shared<FScriptVar>();

		if ( InLeft->varType != InRight->varType )
		{
			return result;
		}

		switch ( InLeft->varType )
		{
		case SVT_String:    result->SetString( InLeft->GetString() + InRight->GetString() ); break;
		case SVT_Int:       result->SetInt( InLeft->GetInt() + InRight->GetInt() ); break;
		case SVT_Bool:      result->SetBool( InLeft->GetBool() + InRight->GetBool() ); break;
		default:
			assert( false );
			return result;
		}

		return result;
	}

	static std::shared_ptr<FScriptVar> Substruct( std::shared_ptr<FScriptVar> InLeft, std::shared_ptr<FScriptVar> InRight )
	{
		std::shared_ptr<FScriptVar>		result = std::make_shared<FScriptVar>();

		if ( InLeft->varType != InRight->varType )
		{
			return result;
		}

		switch ( InLeft->varType )
		{
		case SVT_String:    result->SetString( "Not supported operation" ); break;
		case SVT_Int:       result->SetInt( InLeft->GetInt() - InRight->GetInt() ); break;
		case SVT_Bool:      result->SetBool( InLeft->GetBool() - InRight->GetBool() ); break;
		default:
			assert( false );
			return result;
		}

		return result;
	}

	static std::shared_ptr<FScriptVar> Multiply( std::shared_ptr<FScriptVar> InLeft, std::shared_ptr<FScriptVar> InRight )
	{
		std::shared_ptr<FScriptVar>		result = std::make_shared<FScriptVar>();

		if ( InLeft->varType != InRight->varType )
		{
			return result;
		}

		switch ( InLeft->varType )
		{
		case SVT_String:    result->SetString( "Not supported operation" ); break;
		case SVT_Int:       result->SetInt( InLeft->GetInt() * InRight->GetInt() ); break;
		case SVT_Bool:      result->SetBool( InLeft->GetBool() * InRight->GetBool() ); break;
		default:
			assert( false );
			return result;
		}

		return result;
	}

	static std::shared_ptr<FScriptVar> Divide( std::shared_ptr<FScriptVar> InLeft, std::shared_ptr<FScriptVar> InRight )
	{
		std::shared_ptr<FScriptVar>		result = std::make_shared<FScriptVar>();

		if ( InLeft->varType != InRight->varType )
		{
			return result;
		}

		switch ( InLeft->varType )
		{
		case SVT_String:    result->SetString( "Not supported operation" ); break;
		case SVT_Int:       result->SetInt( InLeft->GetInt() / InRight->GetInt() ); break;
		case SVT_Bool:      result->SetBool( InLeft->GetBool() / InRight->GetBool() ); break;
		default:
			assert( false );
			return result;
		}

		return result;
	}

	bool Compare( std::shared_ptr<FScriptVar> InRight ) const
	{
		if ( varType != InRight->varType )
		{
			return false;
		}

		switch ( varType )
		{
		case SVT_String:    return GetString() == InRight->GetString();
		case SVT_Int:       return GetInt() == InRight->GetInt();
		case SVT_Bool:      return GetBool() == InRight->GetBool();
		default:
			assert( false );
			return false;
		}
	}

	bool More( std::shared_ptr<FScriptVar> InRight ) const
	{
		if ( varType != InRight->varType )
		{
			return false;
		}

		switch ( varType )
		{
		case SVT_String:    return GetString() > InRight->GetString();
		case SVT_Int:       return GetInt() > InRight->GetInt();
		case SVT_Bool:      return GetBool() > InRight->GetBool();
		default:
			assert( false );
			return false;
		}
	}

	bool MoreThen( std::shared_ptr<FScriptVar> InRight ) const
	{
		if ( varType != InRight->varType )
		{
			return false;
		}

		switch ( varType )
		{
		case SVT_String:    return GetString() >= InRight->GetString();
		case SVT_Int:       return GetInt() >= InRight->GetInt();
		case SVT_Bool:      return GetBool() >= InRight->GetBool();
		default:
			assert( false );
			return false;
		}
	}

	bool Less( std::shared_ptr<FScriptVar> InRight ) const
	{
		if ( varType != InRight->varType )
		{
			return false;
		}

		switch ( varType )
		{
		case SVT_String:    return GetString() < InRight->GetString();
		case SVT_Int:       return GetInt() < InRight->GetInt();
		case SVT_Bool:      return GetBool() < InRight->GetBool();
		default:
			assert( false );
			return false;
		}
	}

	bool LessThen( std::shared_ptr<FScriptVar> InRight ) const
	{
		if ( varType != InRight->varType )
		{
			return false;
		}

		switch ( varType )
		{
		case SVT_String:    return GetString() <= InRight->GetString();
		case SVT_Int:       return GetInt() <= InRight->GetInt();
		case SVT_Bool:      return GetBool() <= InRight->GetBool();
		default:
			assert( false );
			return false;
		}
	}

	void Clear()
	{
		if ( !ptr )
		{
			return;
		}

		switch ( varType )
		{
		case SVT_String:        delete static_cast< std::string* >( ptr ); break;
		case SVT_Int:           delete static_cast< int* >( ptr ); break;
		case SVT_Bool:          delete static_cast< bool* >( ptr ); break;
		}

		ptr = nullptr;
	}

	void Set( std::shared_ptr<FScriptVar> InVar )
	{
		switch ( InVar->varType )
		{
		case SVT_String:    SetString( InVar->GetString() );    break;
		case SVT_Int:       SetInt( InVar->GetInt() );          break;
		case SVT_Bool:      SetBool( InVar->GetBool() );          break;
		}
	}

	void SetString( const std::string& InStr )
	{
		if ( varType != SVT_String )
		{
			Clear();
		}

		if ( !IsValid() )
		{
			ptr = new std::string();
			varType = SVT_String;
		}

		*static_cast< std::string* >( ptr ) = InStr;
	}

	void SetInt( int InValue )
	{
		if ( varType != SVT_Int )
		{
			Clear();
		}

		if ( !IsValid() )
		{
			ptr = new int;
			varType = SVT_Int;
		}

		*static_cast< int* >( ptr ) = InValue;
	}

	void SetBool( bool InValue )
	{
		if ( varType != SVT_Bool )
		{
			Clear();
		}

		if ( !IsValid() )
		{
			ptr = new bool;
			varType = SVT_Bool;
		}

		*static_cast< bool* >( ptr ) = InValue;
	}

	EScriptVarType GetType() const
	{
		return varType;
	}

	std::string GetString() const
	{
		if ( !IsValid() || varType != SVT_String )
		{
			return "";
		}
		return *static_cast< std::string* >( ptr );
	}

	int GetInt() const
	{
		if ( !IsValid() || varType != SVT_Int )
		{
			return 0;
		}
		return *static_cast< int* >( ptr );
	}

	bool GetBool() const
	{
		if ( !IsValid() || varType != SVT_Bool )
		{
			return 0;
		}
		return *static_cast< bool* >( ptr );
	}

	bool IsValid() const
	{
		return ptr;
	}

private:
	EScriptVarType      varType;
	void* ptr;
};

struct FFrame
{
	std::vector<std::shared_ptr<FScriptVar>>    vars;
	std::vector<std::shared_ptr<FScriptVar>>    args;
};

// Native functions
typedef void ( *FNativeFunctionFn )( FFrame& );

void execPrint( FFrame& InFrame )
{
	if ( InFrame.args.empty() )
	{
		return;
	}

	for ( int i = 0; i < InFrame.args.size(); ++i )
	{
		std::shared_ptr<FScriptVar>       argItem = InFrame.args[ i ];
		switch ( argItem->GetType() )
		{
		case SVT_String:
			printf( "%s", argItem->GetString().c_str() );
			break;

		case SVT_Int:
			printf( "%i", argItem->GetInt() );
			break;

		case SVT_Bool:
			printf( "%i", argItem->GetBool() );
			break;
		}

		printf( " " );
	}

	printf( "\n" );
}

void execScan( FFrame& InFrame )
{
	if ( InFrame.args.empty() )
	{
		return;
	}

	for ( int i = 0; i < InFrame.args.size(); ++i )
	{
		std::shared_ptr<FScriptVar>   argItem = InFrame.args[ i ];
		switch ( argItem->GetType() )
		{
		case SVT_String:
		{
			std::string     var;
			std::cin >> var;
			argItem->SetString( var );
			break;
		}

		case SVT_Int:
		{
			int var;
			std::cin >> var;
			argItem->SetInt( var );
			break;
		}

		case SVT_Bool:
		{
			bool var;
			std::cin >> var;
			argItem->SetBool( var );
			break;
		}
		}
	}
}

// Returns 'true' if the character is a DELIMITER.
bool isDelimiter( char ch )
{
	if ( ch == ' ' || ch == '+' || ch == '-' || ch == '*' ||
		 ch == '/' || ch == ',' || ch == ';' || ch == '>' ||
		 ch == '<' || ch == '=' || ch == '(' || ch == ')' ||
		 ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '!' )
		return ( true );
	return ( false );
}

// Returns 'true' if the character is an OPERATOR.
bool isOperator( char ch )
{
	if ( ch == '+' || ch == '-' || ch == '*' ||
		 ch == '/' || ch == '>' || ch == '<' ||
		 ch == '=' || ch == '!' )
		return ( true );
	return ( false );
}

// Return 'true' if the character is an ARGS
bool IsArgs( char ch, bool& isEndArgs )
{
	isEndArgs = false;

	if ( ch == '(' )
		return ( true );
	else if ( ch == ')' )
	{
		isEndArgs = true;
		return ( true );
	}

	return ( false );
}

// Return 'true' if the character is an CODE_BODY
bool IsCodeBody( char ch, bool& isEndBody )
{
	isEndBody = false;

	if ( ch == '{' )
		return ( true );
	else if ( ch == '}' )
	{
		isEndBody = true;
		return ( true );
	}

	return ( false );
}

// Return 'true' if the character is an STRING
bool IsStringDelimeter( char ch, bool& isEndString )
{
	if ( ch == '"' )
	{
		isEndString = !isEndString;
		return ( true );
	}

	return ( false );
}

// Return 'true' if the character is and END_CODE_LINE
bool IsEndCodeLine( char ch )
{
	return ch == ';' ? true : false;
}

// Return 'true' if the character is space
bool IsSpace( char ch )
{
	return ch == ' ' ? true : false;
}

// Return 'true' if the character is end of line in file
bool IsEndOfLineInFile( char ch )
{
	return ch == '\n' ? true : false;
}

// Returns 'true' if the string is a VALID IDENTIFIER.
bool validIdentifier( char* str )
{
	if ( str[ 0 ] == '0' || str[ 0 ] == '1' || str[ 0 ] == '2' ||
		 str[ 0 ] == '3' || str[ 0 ] == '4' || str[ 0 ] == '5' ||
		 str[ 0 ] == '6' || str[ 0 ] == '7' || str[ 0 ] == '8' ||
		 str[ 0 ] == '9' || isDelimiter( str[ 0 ] ) == true )
		return ( false );
	return ( true );
}

// Returns 'true' if the string is a KEYWORD.
bool isKeyword( char* str )
{
	if ( !strcmp( str, "if" ) || !strcmp( str, "else" ) ||
		 !strcmp( str, "while" ) || !strcmp( str, "do" ) ||
		 !strcmp( str, "break" ) ||
		 !strcmp( str, "continue" ) || !strcmp( str, "int" )
		 || !strcmp( str, "double" ) || !strcmp( str, "float" )
		 || !strcmp( str, "return" ) || !strcmp( str, "char" )
		 || !strcmp( str, "case" ) || !strcmp( str, "char" )
		 || !strcmp( str, "sizeof" ) || !strcmp( str, "long" )
		 || !strcmp( str, "short" ) || !strcmp( str, "typedef" )
		 || !strcmp( str, "switch" ) || !strcmp( str, "unsigned" )
		 || !strcmp( str, "void" ) || !strcmp( str, "static" )
		 || !strcmp( str, "struct" ) || !strcmp( str, "goto" ) ||
		 !strcmp( str, "string" ) )
		return ( true );
	return ( false );
}
// Returns 'true' if the string is an INTEGER.
bool isInteger( char* str )
{
	int i, len = strlen( str );

	if ( len == 0 )
		return ( false );
	for ( i = 0; i < len; i++ )
	{
		if ( str[ i ] != '0' && str[ i ] != '1' && str[ i ] != '2'
			 && str[ i ] != '3' && str[ i ] != '4' && str[ i ] != '5'
			 && str[ i ] != '6' && str[ i ] != '7' && str[ i ] != '8'
			 && str[ i ] != '9' || ( str[ i ] == '-' && i > 0 ) )
			return ( false );
	}
	return ( true );
}

// Returns 'true' if the string is a REAL NUMBER.
bool isRealNumber( char* str )
{
	int i, len = strlen( str );
	bool hasDecimal = false;

	if ( len == 0 )
		return ( false );
	for ( i = 0; i < len; i++ )
	{
		if ( str[ i ] != '0' && str[ i ] != '1' && str[ i ] != '2'
			 && str[ i ] != '3' && str[ i ] != '4' && str[ i ] != '5'
			 && str[ i ] != '6' && str[ i ] != '7' && str[ i ] != '8'
			 && str[ i ] != '9' && str[ i ] != '.' ||
			 ( str[ i ] == '-' && i > 0 ) )
			return ( false );
		if ( str[ i ] == '.' )
			hasDecimal = true;
	}
	return ( hasDecimal );
}

// Returns 'true' if the string is a STRING
bool isConstString( char* str )
{
	int len = strlen( str );
	if ( len == 0 )
		return ( false );
	return str[ 0 ] == '"' && str[ len - 1 ] == '"';
}

// Extracts the SUBSTRING.
char* subString( char* str, int left, int right )
{
	int i;
	char* subStr = ( char* ) malloc(
		sizeof( char ) * ( right - left + 2 ) );

	for ( i = left; i <= right; i++ )
		subStr[ i - left ] = str[ i ];
	subStr[ right - left + 1 ] = '\0';
	return ( subStr );
}

// Print tabs
void printTabs( int count )
{
	for ( int i = 0; i < count; ++i )
		printf( "\t" );
}

enum ETokenType
{
	TT_Unknown,
	TT_Operator,
	TT_Delimeter,
	TT_Keyword,
	TT_Literal,
	TT_Identifier
};

enum ESubTokenType
{
	STT_None,

	// Operators
	STT_Add,
	STT_Substruct,
	STT_Multiply,
	STT_Divide,
	STT_Less,
	STT_More,
	STT_Appropriate,
	STT_If,
	STT_Else,
	STT_While,
	STT_Not,

	// Delimeters
	STT_BeginArgs,
	STT_EndArgs,
	STT_BeginBody,
	STT_EndBody,
	STT_EndCodeLine,

	// Literals
	STT_Integer,
	STT_Float,
	STT_String,
	STT_Bool,
	STT_User
};

std::string TokenTypeToText( ETokenType InTokenType )
{
	switch ( InTokenType )
	{
	case TT_Operator:       return "Operator";
	case TT_Delimeter:      return "Delimeter";
	case TT_Keyword:        return "Keyword";
	case TT_Literal:        return "Literal";
	case TT_Identifier:     return "Identifier";

	case TT_Unknown:
	default:
		return "Unknown";
	}
}

std::string SubtokenTypeToText( ESubTokenType InSubokenType )
{
	switch ( InSubokenType )
	{
	case STT_Add:               return "Add";
	case STT_Substruct:         return "Substruct";
	case STT_Multiply:          return "Multiply";
	case STT_Divide:            return "Divide";
	case STT_Less:              return "Less";
	case STT_More:              return "More";
	case STT_Appropriate:       return "Appropriate";
	case STT_If:                return "If";
	case STT_Else:              return "Else";
	case STT_While:             return "While";
	case STT_Not:				return "Not";
	case STT_BeginArgs:         return "BeginArgs";
	case STT_EndArgs:           return "EndArgs";
	case STT_BeginBody:         return "BeginBody";
	case STT_EndBody:           return "EndBody";
	case STT_EndCodeLine:       return "EndCodeLine";
	case STT_Integer:           return "Integer";
	case STT_Float:             return "Float";
	case STT_String:            return "String";
	case STT_User:              return "User";

	case STT_None:
	default:
		return "None";
	}
}

EScriptVarType SubtokenTypeToScriptVarType( ESubTokenType InSubokenType )
{
	switch ( InSubokenType )
	{
	case STT_Integer:
		return SVT_Int;

	case STT_String:
		return SVT_String;

	default:
		return SVT_None;
	}
}

ESubTokenType GetSubtokenInOperator( char ch )
{
	assert( isOperator( ch ) );
	switch ( ch )
	{
	case '+':       return STT_Add;
	case '-':       return STT_Substruct;
	case '*':       return STT_Multiply;
	case '/':       return STT_Divide;
	case '>':       return STT_More;
	case '<':       return STT_Less;
	case '=':       return STT_Appropriate;
	case '!':		return STT_Not;
	default:        return STT_None;
	}
}

ESubTokenType GetSubtokenInDelimeter( char ch )
{
	assert( isDelimiter( ch ) );
	switch ( ch )
	{
	case '(':       return STT_BeginArgs;
	case ')':       return STT_EndArgs;
	case '{':       return STT_BeginBody;
	case '}':       return STT_EndBody;
	case ';':       return STT_EndCodeLine;
	default:        return STT_None;
	}
}

ESubTokenType GetSubtokenInKeyword( char* str )
{
	if ( !strcmp( str, "int" ) )
	{
		return STT_Integer;
	}
	else if ( !strcmp( str, "float" ) || !strcmp( str, "double" ) )
	{
		return STT_Float;
	}
	else if ( !strcmp( str, "string" ) )
	{
		return STT_String;
	}
	else if ( !strcmp( str, "if" ) )
	{
		return STT_If;
	}
	else if ( !strcmp( str, "else" ) )
	{
		return STT_Else;
	}
	else if ( !strcmp( str, "while" ) )
	{
		return STT_While;
	}

	return STT_None;
}

struct FToken
{
	struct FTokenKeyFunc
	{
		std::size_t operator()( const FToken& InToken ) const
		{
			return InToken.GetHash();
		}

		bool operator()( const FToken& InA, const FToken& InB ) const
		{
			return InA.GetHash() < InB.GetHash();
		}
	};

	struct FTokenEqualFunc
	{
		bool operator()( const FToken& InA, const FToken& InB ) const
		{
			return InA.GetHash() == InB.GetHash();
		}
	};

	std::size_t GetHash() const
	{
		std::size_t        hash = MemFastHash( originalView.data(), originalView.size() * sizeof( char ) );
		MemFastHash( &type, sizeof( ETokenType ), hash );
		MemFastHash( &subType, sizeof( ESubTokenType ), hash );
		return hash;
	}

	unsigned int        id;             // ID of token
	unsigned int        row;            // Row of start token in file
	unsigned int        column;         // Column of start token in file
	std::string         originalView;   // Original view in source code
	ETokenType          type;           // Type
	ESubTokenType       subType;        // Subtype
};

struct FSemanticItem
{
	FSemanticItem( ETokenType TokenType, ESubTokenType SubTokenType, int InNumPlacableTokens = 0, bool InAnySubType = false, bool InIsOptional = false )
		: type( TokenType ), subType( SubTokenType ), numPlacableTokens( InNumPlacableTokens ), anySubType( InAnySubType ), isOptional( InIsOptional )
	{
	}

	bool Match( const FToken& InToken, bool InIsIgnoreOpetionalFlag = false ) const
	{
		return ( !InIsIgnoreOpetionalFlag && isOptional ) || numPlacableTokens > 0 || numPlacableTokens == -1 || type == InToken.type && ( anySubType || subType == InToken.subType );
	}

	bool                anySubType;
	bool                isOptional;
	int                 numPlacableTokens;  // Number of placable tokens. If -1 end of placable when end delimeter (ex. { and } )
	ETokenType          type;               // Type
	ESubTokenType       subType;            // Subtype
};

struct FDeclFunctionSemanticInfo
{
	FDeclFunctionSemanticInfo()
		: startArgs( 0 ), numArgs( 0 ), startBody( 0 ), numBody( 0 )
	{
	}

	std::string name;       // Function name
	int         startArgs;  // Index to start token arguments
	int         numArgs;    // Number of arguments
	int         startBody;  // Pointer to start token of body function
	int         numBody;    // Number of body tokens
};

struct FCallCodeSemanticInfo
{
	FCallCodeSemanticInfo()
		: isNativeFunction( false ), functionId( -1 ), startArgs( 0 ), numArgs( 0 )
	{
	}

	bool        isNativeFunction;   // Is native function
	int         functionId;         // Function id
	int         startArgs;          // Index to start token arguments
	int         numArgs;            // Number of arguments
};

struct FAllocateVarCodeSemanticInfo
{
	std::string     name;
	EScriptVarType   varType;
};

struct FAssignCodeSemanticInfo
{
	FAssignCodeSemanticInfo()
		: leftVarId( 0 ), rightVarId( 0 ), isRightConstVar( false )
	{
	}

	int     leftVarId;
	int     rightVarId;
	bool    isRightConstVar;
};

struct FArithmeticCodeSemanticInfo
{
	FArithmeticCodeSemanticInfo()
		: leftVarId( 0 ), rightVarId( 0 ), resultToVarId( 0 ), isRightConstVar( false )
	{
	}

	int     leftVarId;
	int     rightVarId;
	int		resultToVarId;
	bool    isRightConstVar;
};

struct FIfCodeSemanticInfo
{
	int     startIfArgs;
	int     numIfArgs;
	int     startBody;
	int     numBody;
	int     startElseBody;
	int     numElseBody;
};

struct FWhileCodeSemanticInfo
{
	int     startIfArgs;
	int     numIfArgs;
	int     startBody;
	int     numBody;
};

struct FCompareSemanticInfo
{
	FCompareSemanticInfo()
		: leftVarId( 0 ), rightVarId( 0 ), isRightConstVar( false ), isLeftConstVar( false )
	{
	}

	int     leftVarId;
	int     rightVarId;
	bool    isLeftConstVar;
	bool    isRightConstVar;
};

class FFunction
{
public:
	FFunction( const std::string& InName, const std::vector<int>& InCode )
		: name( InName ), code( InCode )
	{
	}

	FFunction( const FFunction& InCopy )
		: name( InCopy.name ), code( InCopy.code )
	{
	}

	void Execute( FFrame& InFrame );

	FFunction& operator=( const FFunction& InCopy )
	{
		name = InCopy.name;
		code = InCopy.code;
		return *this;
	}

	const std::string& GetName() const
	{
		return name;
	}

private:
	std::string         name;
	std::vector<int>	code;
};

struct FNativeFunction
{
	std::string             name;
	FNativeFunctionFn       functionFn;
};

class FCTranslator
{
public:
	// Load source code from file
	bool LoadFromFile( const std::string& InPath )
	{
		std::string     buffer;
		std::ifstream   file( InPath );
		if ( !file.is_open() )
		{
			printf( "Error: Failed loading source code '%s'\n", InPath.c_str() );
			return false;
		}

		// Read all file to buffer
		std::getline( file, buffer, '\0' );

		// Parse code
		std::string     errorMsg;
		bool    bResult = Parse( ( char* ) buffer.data(), errorMsg );
		if ( !bResult )
		{
			printf( "Error: %s", errorMsg.c_str() );
			return false;
		}

		printf( "Code seccussed loaded and parsed\n" );
		return true;
	}

	// Print to console dump all parsed tokens
	void DumpTokens()
	{
		if ( tokens.empty() )
		{
			printf( "Error: file not loaded or empty" );
			return;
		}

		printf( "ID\t\tRow\t\tColumn\t\tOriginal View\t\tType\t\t\tSubtype\n" );
		for ( int i = 0; i < tokens.size(); i++ )
		{
			const FToken& token = tokens[ i ];
			printf( "%i\t\t%i\t\t%i\t\t%s\t\t\t%-10s\t\t%s\n",
					token.id,
					token.row,
					token.column,
					token.originalView.c_str(),
					TokenTypeToText( token.type ).c_str(),
					SubtokenTypeToText( token.subType ).c_str() );
		}
	}

	// Print to console dump all parsed user identifiers
	void DumpUserIdentifiers()
	{
		if ( userIdentifiers.empty() )
		{
			printf( "Error: file not loaded or empty" );
			return;
		}

		printf( "ID\t\tRow\t\tColumn\t\tOriginal View\t\tType\t\t\tSubtype\n" );
		for ( auto it = userIdentifiers.begin(); it != userIdentifiers.end(); it++ )
		{
			const FToken& token = it->first;
			printf( "%i\t\t%i\t\t%i\t\t%s\t\t\t%-10s\t\t%s\n",
					token.id,
					token.row,
					token.column,
					token.originalView.c_str(),
					TokenTypeToText( token.type ).c_str(),
					SubtokenTypeToText( token.subType ).c_str() );
		}
	}

	// Print to console dump all functions
	void DumpFunctions()
	{
		if ( !functions.empty() )
		{
			printf( "Functions:\n" );
			for ( int i = 0; i < functions.size(); ++i )
			{
				printf( "ID: %i, Name: %s\n", i, functions[ i ].GetName().c_str() );
			}
		}
		else
		{
			printf( "Functions: empty\n" );
		}

		if ( !nativeFunctions.empty() )
		{
			printf( "\nNative functions:\n" );
			for ( int i = 0; i < nativeFunctions.size(); ++i )
			{
				printf( "ID: %i, Name: %s\n", i, nativeFunctions[ i ].name.c_str() );
			}
		}
		else
		{
			printf( "\nNative functions: empty\n" );
		}
	}

	void Init()
	{
		// Register native functions
		RegisterNativeFunction( "print", &execPrint );
		RegisterNativeFunction( "scan", &execScan );
	}

	void ExecuteFunction( const std::string& InFuncName, FFrame& InFrame )
	{
		auto    itFunc = functionNameToID.find( InFuncName );
		if ( itFunc != functionNameToID.end() )
		{
			functions[ itFunc->second ].Execute( InFrame );
			return;
		}

		auto    itNativeFunc = nativeFunctionNameToID.find( InFuncName );
		if ( itNativeFunc != nativeFunctionNameToID.end() )
		{
			nativeFunctions[ itNativeFunc->second ].functionFn( InFrame );
			return;
		}
	}

	void ExecuteFunction( int InFuncId, bool InIsNative, FFrame& InFrame )
	{
		if ( InIsNative )
		{
			assert( !nativeFunctions.empty() && InFuncId >= 0 && InFuncId < nativeFunctions.size() );
			nativeFunctions[ InFuncId ].functionFn( InFrame );
		}
		else
		{
			assert( !functions.empty() && InFuncId >= 0 && InFuncId < functions.size() );
			functions[ InFuncId ].Execute( InFrame );
		}
	}

	std::shared_ptr<FScriptVar> GetVarConstant( int InVarId )
	{
		assert( !varConstants.empty() && InVarId >= 0 && InVarId < varConstants.size() );
		return varConstants[ InVarId ];
	}

private:
	// Parse
	bool Parse( char* str, std::string& OutErrorStr )
	{
		// Parse string
		unsigned int lastID = 0;
		int left = 0, right = 0;
		int len = strlen( str );

		int row = 1;
		int column = 1;
		int startRow = 0;
		bool isEndString = true;

		while ( right <= len && left <= right )
		{
			if ( str[ right ] == '\n' )
			{
				str[ right ] = ' ';
				row++;
				column = 0;
				startRow = right + 1;
			}
			else if ( str[ right ] == '\t' )
			{
				str[ right ] = ' ';
			}

			if ( IsStringDelimeter( str[ right ], isEndString ) || !isEndString )
			{
				right++;
				continue;
			}
			else
			{
				if ( isDelimiter( str[ right ] ) == false )
					right++;
			}

			if ( isDelimiter( str[ right ] ) == true && left == right )
			{
				bool    isEndDelimeter_DEPRECATED = false;

				if ( isOperator( str[ right ] ) == true )
				{
					FToken      token;
					token.id = lastID;
					token.row = row;
					token.column = right - startRow + 1;
					token.originalView = str[ right ];
					token.type = TT_Operator;
					token.subType = GetSubtokenInOperator( str[ right ] );
					lastID++;
					tokens.push_back( token );
				}
				else if ( IsArgs( str[ right ], isEndDelimeter_DEPRECATED ) == true )
				{
					FToken      token;
					token.id = lastID;
					token.row = row;
					token.column = right - startRow + 1;
					token.originalView = str[ right ];
					token.type = TT_Delimeter;
					token.subType = GetSubtokenInDelimeter( str[ right ] );
					lastID++;
					tokens.push_back( token );
				}
				else if ( IsCodeBody( str[ right ], isEndDelimeter_DEPRECATED ) == true )
				{
					FToken      token;
					token.id = lastID;
					token.row = row;
					token.column = right - startRow + 1;
					token.originalView = str[ right ];
					token.type = TT_Delimeter;
					token.subType = GetSubtokenInDelimeter( str[ right ] );
					++lastID;
					tokens.push_back( token );
				}
				else if ( IsEndCodeLine( str[ right ] ) )
				{
					FToken      token;
					token.id = lastID;
					token.row = row;
					token.column = right - startRow + 1;
					token.originalView = str[ right ];
					token.type = TT_Delimeter;
					token.subType = GetSubtokenInDelimeter( str[ right ] );
					lastID++;
					tokens.push_back( token );
				}

				right++;
				left = right;
			}
			else if ( isDelimiter( str[ right ] ) == true && left != right
					  || ( right == len && left != right ) )
			{
				char* subStr = subString( str, left, right - 1 );

				if ( isKeyword( subStr ) == true )
				{
					FToken      token;
					token.id = lastID;
					token.row = row;
					token.column = left - startRow + 1;
					token.originalView = subStr;
					token.type = TT_Keyword;
					token.subType = GetSubtokenInKeyword( subStr );
					++lastID;
					tokens.push_back( token );
				}

				else if ( isInteger( subStr ) == true )
				{
					FToken      token;
					token.id = lastID;
					token.row = row;
					token.column = left - startRow + 1;
					token.originalView = subStr;
					token.type = TT_Literal;
					token.subType = STT_Integer;
					lastID++;
					tokens.push_back( token );
				}

				else if ( isRealNumber( subStr ) == true )
				{
					FToken      token;
					token.id = lastID;
					token.row = row;
					token.column = left - startRow + 1;
					token.originalView = subStr;
					token.type = TT_Literal;
					token.subType = STT_Float;
					++lastID;
					tokens.push_back( token );
				}

				else if ( isConstString( subStr ) == true )
				{
					// Remove from subStr '"'
					subStr[ strlen( subStr ) - 1 ] = '\0';
					++subStr;

					FToken      token;
					token.id = lastID;
					token.row = row;
					token.column = left - startRow + 1;
					token.originalView = subStr;
					token.type = TT_Literal;
					token.subType = STT_String;
					++lastID;
					tokens.push_back( token );
				}

				else if ( validIdentifier( subStr ) == true
						  && isDelimiter( str[ right - 1 ] ) == false )
				{
					FToken      token;
					token.row = row;
					token.column = left - startRow + 1;
					token.originalView = subStr;
					token.type = TT_Identifier;
					token.subType = STT_User;

					bool        bResult = FindIDUserIdentifiers( token, token.id );
					if ( !bResult )
					{
						token.id = lastID;
						userIdentifiers[ token ] = token.id;
						++lastID;
					}

					tokens.push_back( token );
				}

				else if ( validIdentifier( subStr ) == false
						  && isDelimiter( str[ right - 1 ] ) == false )
				{
					OutErrorStr = std::string( "(" ) + std::to_string( row ) + ":" + std::to_string( left - startRow + 1 ) + "): " + std::string( subStr ) + " is not a valid identifier";
					return false;
				}
				left = right;
			}
		}

		assert( !tokens.empty() );

		// Token Analysis
		FDeclFunctionSemanticInfo       declFunctionSemanticInfo;

		int     indexToken = 0;
		while ( indexToken < tokens.size() )
		{
			if ( IsDeclFunctionSemantic( indexToken, declFunctionSemanticInfo ) )
			{
				std::vector<int>						byteCode;
				std::unordered_map<std::string, int>    varNameToID;
				std::unordered_map<std::string, int>    argsVarNameToID;
				FAllocateVarCodeSemanticInfo            allocateVarCodeSemanticInfo;

				int     indexArgToken = declFunctionSemanticInfo.startArgs;
				int     lastIndexArgToken = declFunctionSemanticInfo.startArgs + declFunctionSemanticInfo.numArgs;
				while ( indexArgToken < lastIndexArgToken )
				{
					if ( IsDeclVarInArgsSemantic( indexArgToken, lastIndexArgToken - indexArgToken, allocateVarCodeSemanticInfo ) )
					{
						int     numArgVars = argsVarNameToID.size();
						argsVarNameToID[ allocateVarCodeSemanticInfo.name ] = numArgVars;
					}
				}

				int     indexBodyToken = declFunctionSemanticInfo.startBody;
				int     lastIndexBodyToken = declFunctionSemanticInfo.startBody + declFunctionSemanticInfo.numBody;
				if ( !ParseBody( indexBodyToken, lastIndexBodyToken, varNameToID, argsVarNameToID, byteCode ) )
				{
					assert( false );
					return false;
				}

				RegisterFunction( FFunction( declFunctionSemanticInfo.name, byteCode ) );
			}
			else
			{
				++indexToken;
			}
		}

		return true;
	}

	bool ParseBody( int& InOutIndexBodyToken, int InLastIndexBodyToken, std::unordered_map<std::string, int>& InVarsNameToID, std::unordered_map<std::string, int>& InArgVarsNameToID, std::vector<int>& OutByteCode )
	{
		FCallCodeSemanticInfo                   callCodeSemanticInfo;
		FAllocateVarCodeSemanticInfo            allocateVarCodeSemanticInfo;
		FAssignCodeSemanticInfo                 assignCodeSemanticInfo;
		FIfCodeSemanticInfo                     ifCodeSemanticInfo;
		FWhileCodeSemanticInfo                  whileCodeSemanticInfo;
		FArithmeticCodeSemanticInfo				arithmeticCodeSemanticInfo;

		while ( InOutIndexBodyToken < InLastIndexBodyToken )
		{
			int		tmpIndexBodyToke = InOutIndexBodyToken;
			int     numTokens = InLastIndexBodyToken - InOutIndexBodyToken;
			if ( IsCallCodeSemantic( tmpIndexBodyToke, numTokens, callCodeSemanticInfo ) )
			{
				OutByteCode.push_back( callCodeSemanticInfo.isNativeFunction ? Op_NativeCall : Op_Call );
				OutByteCode.push_back( callCodeSemanticInfo.functionId );
				OutByteCode.push_back( callCodeSemanticInfo.numArgs );

				for ( int indexArgToken = callCodeSemanticInfo.startArgs; indexArgToken < callCodeSemanticInfo.startArgs + callCodeSemanticInfo.numArgs; ++indexArgToken )
				{
					const FToken& token = tokens[ indexArgToken ];
					int                 varId = 0;
					std::shared_ptr<FScriptVar>          scriptVar = std::make_shared<FScriptVar>();

					if ( token.type == TT_Literal )
					{
						switch ( token.subType )
						{
						case STT_String:
							scriptVar->SetString( token.originalView );
							break;

						case STT_Integer:
							scriptVar->SetInt( atoi( token.originalView.c_str() ) );
							break;

						case STT_Bool:
							scriptVar->SetBool( atoi( token.originalView.c_str() ) );
							break;

						default:
							assert( false );
							return false;
							break;
						}

						RegisterVarConstant( scriptVar, &varId );
						OutByteCode.push_back( SVF_Const );
						OutByteCode.push_back( varId );
					}
					else if ( token.type == TT_Identifier && token.subType == STT_User )
					{
						auto    itArgVar = InArgVarsNameToID.find( token.originalView );
						if ( itArgVar != InArgVarsNameToID.end() )
						{
							OutByteCode.push_back( SVF_Arg );
							OutByteCode.push_back( itArgVar->second );
							continue;
						}

						auto    itVar = InVarsNameToID.find( token.originalView );
						if ( itVar != InVarsNameToID.end() )
						{
							OutByteCode.push_back( SVF_User );
							OutByteCode.push_back( itVar->second );
							continue;
						}

						assert( false );
						return false;
					}
					else
					{
						assert( false );
						return false;
					}
				}
			}
			else if ( IsAllocateVarCodeSemantic( tmpIndexBodyToke, numTokens, allocateVarCodeSemanticInfo ) )
			{
				OutByteCode.push_back( Op_AllocateVar );
				OutByteCode.push_back( allocateVarCodeSemanticInfo.varType );

				int     numVars = InVarsNameToID.size();
				InVarsNameToID[ allocateVarCodeSemanticInfo.name ] = numVars;
			}
			else if ( IsAssignCodeSemantic( tmpIndexBodyToke, numTokens, InVarsNameToID, InArgVarsNameToID, assignCodeSemanticInfo ) )
			{
				int     varFlag = assignCodeSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
				if ( varFlag == SVF_User )
				{
					for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
					{
						if ( it->second == assignCodeSemanticInfo.rightVarId )
						{
							varFlag = SVF_Arg;
							break;
						}
					}
				}

				OutByteCode.push_back( Op_Assign );
				OutByteCode.push_back( assignCodeSemanticInfo.leftVarId );
				OutByteCode.push_back( varFlag );
				OutByteCode.push_back( assignCodeSemanticInfo.rightVarId );
			}
			else if ( IsIfCodeSemantic( tmpIndexBodyToke, numTokens, InVarsNameToID, InArgVarsNameToID, ifCodeSemanticInfo ) )
			{
				FCompareSemanticInfo        compareSemanticInfo;

				int     indexArgToken = ifCodeSemanticInfo.startIfArgs;
				int     lastIndexArgToken = ifCodeSemanticInfo.startIfArgs + ifCodeSemanticInfo.numIfArgs;
				int     numArgTokens = lastIndexArgToken - indexArgToken;
				while ( indexArgToken < lastIndexArgToken )
				{
					if ( IsCompareCodeSemantic( indexArgToken, numArgTokens, InVarsNameToID, InArgVarsNameToID, compareSemanticInfo ) )
					{
						int     leftVarFlag = compareSemanticInfo.isLeftConstVar ? SVF_Const : SVF_User;
						if ( leftVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									leftVarFlag = SVF_Arg;
									break;
								}
							}
						}

						int     rightVarFlag = compareSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
						if ( rightVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									rightVarFlag = SVF_Arg;
									break;
								}
							}
						}

						OutByteCode.push_back( Op_Compare );
						OutByteCode.push_back( leftVarFlag );
						OutByteCode.push_back( compareSemanticInfo.leftVarId );
						OutByteCode.push_back( rightVarFlag );
						OutByteCode.push_back( compareSemanticInfo.rightVarId );
					}
					else if ( IsNotCompareCodeSemantic( indexArgToken, numArgTokens, InVarsNameToID, InArgVarsNameToID, compareSemanticInfo ) )
					{
						int     leftVarFlag = compareSemanticInfo.isLeftConstVar ? SVF_Const : SVF_User;
						if ( leftVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									leftVarFlag = SVF_Arg;
									break;
								}
							}
						}

						int     rightVarFlag = compareSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
						if ( rightVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									rightVarFlag = SVF_Arg;
									break;
								}
							}
						}

						OutByteCode.push_back( Op_NotCompare );
						OutByteCode.push_back( leftVarFlag );
						OutByteCode.push_back( compareSemanticInfo.leftVarId );
						OutByteCode.push_back( rightVarFlag );
						OutByteCode.push_back( compareSemanticInfo.rightVarId );
					}
					else if ( IsMoreCodeSemantic( indexArgToken, numArgTokens, InVarsNameToID, InArgVarsNameToID, compareSemanticInfo ) )
					{
						int     leftVarFlag = compareSemanticInfo.isLeftConstVar ? SVF_Const : SVF_User;
						if ( leftVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									leftVarFlag = SVF_Arg;
									break;
								}
							}
						}

						int     rightVarFlag = compareSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
						if ( rightVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									rightVarFlag = SVF_Arg;
									break;
								}
							}
						}

						OutByteCode.push_back( Op_More );
						OutByteCode.push_back( leftVarFlag );
						OutByteCode.push_back( compareSemanticInfo.leftVarId );
						OutByteCode.push_back( rightVarFlag );
						OutByteCode.push_back( compareSemanticInfo.rightVarId );
					}
					else if ( IsMoreThenCodeSemantic( indexArgToken, numArgTokens, InVarsNameToID, InArgVarsNameToID, compareSemanticInfo ) )
					{
						int     leftVarFlag = compareSemanticInfo.isLeftConstVar ? SVF_Const : SVF_User;
						if ( leftVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									leftVarFlag = SVF_Arg;
									break;
								}
							}
						}

						int     rightVarFlag = compareSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
						if ( rightVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									rightVarFlag = SVF_Arg;
									break;
								}
							}
						}

						OutByteCode.push_back( Op_MoreThen );
						OutByteCode.push_back( leftVarFlag );
						OutByteCode.push_back( compareSemanticInfo.leftVarId );
						OutByteCode.push_back( rightVarFlag );
						OutByteCode.push_back( compareSemanticInfo.rightVarId );
					}
					else if ( IsLessThenCodeSemantic( indexArgToken, numArgTokens, InVarsNameToID, InArgVarsNameToID, compareSemanticInfo ) )
					{
						int     leftVarFlag = compareSemanticInfo.isLeftConstVar ? SVF_Const : SVF_User;
						if ( leftVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									leftVarFlag = SVF_Arg;
									break;
								}
							}
						}

						int     rightVarFlag = compareSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
						if ( rightVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									rightVarFlag = SVF_Arg;
									break;
								}
							}
						}

						OutByteCode.push_back( Op_LessThen );
						OutByteCode.push_back( leftVarFlag );
						OutByteCode.push_back( compareSemanticInfo.leftVarId );
						OutByteCode.push_back( rightVarFlag );
						OutByteCode.push_back( compareSemanticInfo.rightVarId );
					}
					else if ( IsLessCodeSemantic( indexArgToken, numArgTokens, InVarsNameToID, InArgVarsNameToID, compareSemanticInfo ) )
					{
						int     leftVarFlag = compareSemanticInfo.isLeftConstVar ? SVF_Const : SVF_User;
						if ( leftVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									leftVarFlag = SVF_Arg;
									break;
								}
							}
						}

						int     rightVarFlag = compareSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
						if ( rightVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									rightVarFlag = SVF_Arg;
									break;
								}
							}
						}

						OutByteCode.push_back( Op_Less );
						OutByteCode.push_back( leftVarFlag );
						OutByteCode.push_back( compareSemanticInfo.leftVarId );
						OutByteCode.push_back( rightVarFlag );
						OutByteCode.push_back( compareSemanticInfo.rightVarId );
					}
				}

				OutByteCode.push_back( Op_JumpNotEqual );
				OutByteCode.push_back( -1 );        // Need placable after generate byte code for this 'if'
				int     codeOffsetElse = OutByteCode.size() - 1;
				int     codeOffsetJumpInBodyIf = -1;

				int     indexBodyToken = ifCodeSemanticInfo.startBody;
				int     lastIndexBodyToken = ifCodeSemanticInfo.startBody + ifCodeSemanticInfo.numBody;
				if ( !ParseBody( indexBodyToken, lastIndexBodyToken, InVarsNameToID, InArgVarsNameToID, OutByteCode ) )
				{
					assert( false );
					return false;
				}

				if ( ifCodeSemanticInfo.numElseBody > 0 )
				{
					OutByteCode.push_back( Op_Jump );
					OutByteCode.push_back( -1 );
					codeOffsetJumpInBodyIf = OutByteCode.size() - 1;
				}

				OutByteCode[ codeOffsetElse ] = OutByteCode.size();
				OutByteCode.push_back( Op_Nope );

				if ( ifCodeSemanticInfo.numElseBody > 0 )
				{
					indexBodyToken = ifCodeSemanticInfo.startElseBody;
					lastIndexBodyToken = ifCodeSemanticInfo.startElseBody + ifCodeSemanticInfo.numElseBody;
					if ( !ParseBody( indexBodyToken, lastIndexBodyToken, InVarsNameToID, InArgVarsNameToID, OutByteCode ) )
					{
						assert( false );
						return false;
					}

					OutByteCode[ codeOffsetJumpInBodyIf ] = OutByteCode.size();
					OutByteCode.push_back( Op_Nope );
				}
			}
			else if ( IsWhileCodeSemantic( tmpIndexBodyToke, numTokens, InVarsNameToID, InArgVarsNameToID, whileCodeSemanticInfo ) )
			{
				FCompareSemanticInfo        compareSemanticInfo;
				int							codeOffsetWhile = OutByteCode.size();

				int     indexArgToken = whileCodeSemanticInfo.startIfArgs;
				int     lastIndexArgToken = whileCodeSemanticInfo.startIfArgs + whileCodeSemanticInfo.numIfArgs;
				int     numArgTokens = lastIndexArgToken - indexArgToken;
				while ( indexArgToken < lastIndexArgToken )
				{
					if ( IsCompareCodeSemantic( indexArgToken, numArgTokens, InVarsNameToID, InArgVarsNameToID, compareSemanticInfo ) )
					{
						int     leftVarFlag = compareSemanticInfo.isLeftConstVar ? SVF_Const : SVF_User;
						if ( leftVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									leftVarFlag = SVF_Arg;
									break;
								}
							}
						}

						int     rightVarFlag = compareSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
						if ( rightVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									rightVarFlag = SVF_Arg;
									break;
								}
							}
						}

						OutByteCode.push_back( Op_Compare );
						OutByteCode.push_back( leftVarFlag );
						OutByteCode.push_back( compareSemanticInfo.leftVarId );
						OutByteCode.push_back( rightVarFlag );
						OutByteCode.push_back( compareSemanticInfo.rightVarId );
					}
					else if ( IsNotCompareCodeSemantic( indexArgToken, numArgTokens, InVarsNameToID, InArgVarsNameToID, compareSemanticInfo ) )
					{
						int     leftVarFlag = compareSemanticInfo.isLeftConstVar ? SVF_Const : SVF_User;
						if ( leftVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									leftVarFlag = SVF_Arg;
									break;
								}
							}
						}

						int     rightVarFlag = compareSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
						if ( rightVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									rightVarFlag = SVF_Arg;
									break;
								}
							}
						}

						OutByteCode.push_back( Op_NotCompare );
						OutByteCode.push_back( leftVarFlag );
						OutByteCode.push_back( compareSemanticInfo.leftVarId );
						OutByteCode.push_back( rightVarFlag );
						OutByteCode.push_back( compareSemanticInfo.rightVarId );
					}
					else if ( IsMoreCodeSemantic( indexArgToken, numArgTokens, InVarsNameToID, InArgVarsNameToID, compareSemanticInfo ) )
					{
						int     leftVarFlag = compareSemanticInfo.isLeftConstVar ? SVF_Const : SVF_User;
						if ( leftVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									leftVarFlag = SVF_Arg;
									break;
								}
							}
						}

						int     rightVarFlag = compareSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
						if ( rightVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									rightVarFlag = SVF_Arg;
									break;
								}
							}
						}

						OutByteCode.push_back( Op_More );
						OutByteCode.push_back( leftVarFlag );
						OutByteCode.push_back( compareSemanticInfo.leftVarId );
						OutByteCode.push_back( rightVarFlag );
						OutByteCode.push_back( compareSemanticInfo.rightVarId );
					}
					else if ( IsMoreThenCodeSemantic( indexArgToken, numArgTokens, InVarsNameToID, InArgVarsNameToID, compareSemanticInfo ) )
					{
						int     leftVarFlag = compareSemanticInfo.isLeftConstVar ? SVF_Const : SVF_User;
						if ( leftVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									leftVarFlag = SVF_Arg;
									break;
								}
							}
						}

						int     rightVarFlag = compareSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
						if ( rightVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									rightVarFlag = SVF_Arg;
									break;
								}
							}
						}

						OutByteCode.push_back( Op_MoreThen );
						OutByteCode.push_back( leftVarFlag );
						OutByteCode.push_back( compareSemanticInfo.leftVarId );
						OutByteCode.push_back( rightVarFlag );
						OutByteCode.push_back( compareSemanticInfo.rightVarId );
					}
					else if ( IsLessThenCodeSemantic( indexArgToken, numArgTokens, InVarsNameToID, InArgVarsNameToID, compareSemanticInfo ) )
					{
						int     leftVarFlag = compareSemanticInfo.isLeftConstVar ? SVF_Const : SVF_User;
						if ( leftVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									leftVarFlag = SVF_Arg;
									break;
								}
							}
						}

						int     rightVarFlag = compareSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
						if ( rightVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									rightVarFlag = SVF_Arg;
									break;
								}
							}
						}

						OutByteCode.push_back( Op_LessThen );
						OutByteCode.push_back( leftVarFlag );
						OutByteCode.push_back( compareSemanticInfo.leftVarId );
						OutByteCode.push_back( rightVarFlag );
						OutByteCode.push_back( compareSemanticInfo.rightVarId );
					}
					else if ( IsLessCodeSemantic( indexArgToken, numArgTokens, InVarsNameToID, InArgVarsNameToID, compareSemanticInfo ) )
					{
						int     leftVarFlag = compareSemanticInfo.isLeftConstVar ? SVF_Const : SVF_User;
						if ( leftVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									leftVarFlag = SVF_Arg;
									break;
								}
							}
						}

						int     rightVarFlag = compareSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
						if ( rightVarFlag == SVF_User )
						{
							for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
							{
								if ( it->second == assignCodeSemanticInfo.rightVarId )
								{
									rightVarFlag = SVF_Arg;
									break;
								}
							}
						}

						OutByteCode.push_back( Op_Less );
						OutByteCode.push_back( leftVarFlag );
						OutByteCode.push_back( compareSemanticInfo.leftVarId );
						OutByteCode.push_back( rightVarFlag );
						OutByteCode.push_back( compareSemanticInfo.rightVarId );
					}
				}

				OutByteCode.push_back( Op_JumpNotEqual );
				OutByteCode.push_back( -1 );        // Need placable after generate byte code for this 'if'
				int     codeOffsetElse = OutByteCode.size() - 1;

				int     indexBodyToken = whileCodeSemanticInfo.startBody;
				int     lastIndexBodyToken = whileCodeSemanticInfo.startBody + whileCodeSemanticInfo.numBody;
				if ( !ParseBody( indexBodyToken, lastIndexBodyToken, InVarsNameToID, InArgVarsNameToID, OutByteCode ) )
				{
					assert( false );
					return false;
				}

				OutByteCode.push_back( Op_Jump );
				OutByteCode.push_back( codeOffsetWhile );

				OutByteCode[ codeOffsetElse ] = OutByteCode.size();
				OutByteCode.push_back( Op_Nope );
			}
			else if ( IsAddCodeSemantic( tmpIndexBodyToke, numTokens, InVarsNameToID, InArgVarsNameToID, arithmeticCodeSemanticInfo ) )
			{
				int     varFlag = arithmeticCodeSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
				if ( varFlag == SVF_User )
				{
					for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
					{
						if ( it->second == arithmeticCodeSemanticInfo.rightVarId )
						{
							varFlag = SVF_Arg;
							break;
						}
					}
				}

				OutByteCode.push_back( Op_Add );
				OutByteCode.push_back( arithmeticCodeSemanticInfo.leftVarId );
				OutByteCode.push_back( varFlag );
				OutByteCode.push_back( arithmeticCodeSemanticInfo.rightVarId );

				OutByteCode.push_back( Op_Assign );
				OutByteCode.push_back( arithmeticCodeSemanticInfo.resultToVarId );
				OutByteCode.push_back( SVF_Register );
				OutByteCode.push_back( SR_AX );
			}
			else if ( IsSubstructCodeSemantic( tmpIndexBodyToke, numTokens, InVarsNameToID, InArgVarsNameToID, arithmeticCodeSemanticInfo ) )
			{
				int     varFlag = arithmeticCodeSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
				if ( varFlag == SVF_User )
				{
					for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
					{
						if ( it->second == arithmeticCodeSemanticInfo.rightVarId )
						{
							varFlag = SVF_Arg;
							break;
						}
					}
				}

				OutByteCode.push_back( Op_Substruct );
				OutByteCode.push_back( arithmeticCodeSemanticInfo.leftVarId );
				OutByteCode.push_back( varFlag );
				OutByteCode.push_back( arithmeticCodeSemanticInfo.rightVarId );

				OutByteCode.push_back( Op_Assign );
				OutByteCode.push_back( arithmeticCodeSemanticInfo.resultToVarId );
				OutByteCode.push_back( SVF_Register );
				OutByteCode.push_back( SR_AX );
			}
			else if ( IsMultiplyCodeSemantic( tmpIndexBodyToke, numTokens, InVarsNameToID, InArgVarsNameToID, arithmeticCodeSemanticInfo ) )
			{
				int     varFlag = arithmeticCodeSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
				if ( varFlag == SVF_User )
				{
					for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
					{
						if ( it->second == arithmeticCodeSemanticInfo.rightVarId )
						{
							varFlag = SVF_Arg;
							break;
						}
					}
				}

				OutByteCode.push_back( Op_Multiply );
				OutByteCode.push_back( arithmeticCodeSemanticInfo.leftVarId );
				OutByteCode.push_back( varFlag );
				OutByteCode.push_back( arithmeticCodeSemanticInfo.rightVarId );

				OutByteCode.push_back( Op_Assign );
				OutByteCode.push_back( arithmeticCodeSemanticInfo.resultToVarId );
				OutByteCode.push_back( SVF_Register );
				OutByteCode.push_back( SR_AX );
			}
			else if ( IsDivideCodeSemantic( tmpIndexBodyToke, numTokens, InVarsNameToID, InArgVarsNameToID, arithmeticCodeSemanticInfo ) )
			{
				int     varFlag = arithmeticCodeSemanticInfo.isRightConstVar ? SVF_Const : SVF_User;
				if ( varFlag == SVF_User )
				{
					for ( auto it = InArgVarsNameToID.begin(), itEnd = InArgVarsNameToID.end(); it != itEnd; ++it )
					{
						if ( it->second == arithmeticCodeSemanticInfo.rightVarId )
						{
							varFlag = SVF_Arg;
							break;
						}
					}
				}

				OutByteCode.push_back( Op_Divide );
				OutByteCode.push_back( arithmeticCodeSemanticInfo.leftVarId );
				OutByteCode.push_back( varFlag );
				OutByteCode.push_back( arithmeticCodeSemanticInfo.rightVarId );

				OutByteCode.push_back( Op_Assign );
				OutByteCode.push_back( arithmeticCodeSemanticInfo.resultToVarId );
				OutByteCode.push_back( SVF_Register );
				OutByteCode.push_back( SR_AX );
			}
			else
			{
				assert( false );
				return false;
			}

			InOutIndexBodyToken = tmpIndexBodyToke;
		}

		return true;
	}

	bool FindIDUserIdentifiers( const FToken& InToken, unsigned int& OutId )
	{
		auto        it = userIdentifiers.find( InToken );
		if ( it == userIdentifiers.end() )
		{
			OutId = -1;
			return false;
		}

		OutId = it->second;
		return true;
	}

	bool IsAddCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FArithmeticCodeSemanticInfo& OutArithmeticCodeSemanticInfo )
	{
		static std::vector< std::vector<FSemanticItem> >       semantics =
		{
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Add ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Add ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_Add ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_Add ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     startToken = InOutTokenIndex;
		int     endToken = startToken + InNumTokens;
		int     currentToken = startToken;
		bool    result = true;
		bool	isAref = false;
		for ( int indexSemantics = 0; indexSemantics < semantics.size(); ++indexSemantics )
		{
			bool                                    isRightVar = false;
			const std::vector<FSemanticItem>& semantic = semantics[ indexSemantics ];
			result = true;

			currentToken = startToken;
			for ( int indexSemantic = 0; currentToken <= endToken && indexSemantic < semantic.size(); ++currentToken )
			{
				const FSemanticItem& semanticItem = semantic[ indexSemantic ];
				if ( semanticItem.Match( tokens[ currentToken ] ) )
				{
					if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
					{
						bool    isSeccussed = false;
						{
							auto    itArgVarId = InArgVarsNameToID.find( tokens[ currentToken ].originalView );
							if ( itArgVarId != InArgVarsNameToID.end() )
							{
								if ( isAref )
								{
									if ( !isRightVar )
									{
										OutArithmeticCodeSemanticInfo.leftVarId = itArgVarId->second;
									}
									else
									{
										OutArithmeticCodeSemanticInfo.rightVarId = itArgVarId->second;
									}
								}
								else
								{
									OutArithmeticCodeSemanticInfo.resultToVarId = itArgVarId->second;
								}

								isSeccussed = true;
							}
							else
							{
								auto    itVarId = InVarsNameToID.find( tokens[ currentToken ].originalView );
								if ( itVarId != InVarsNameToID.end() )
								{
									if ( isAref )
									{
										if ( !isRightVar )
										{
											OutArithmeticCodeSemanticInfo.leftVarId = itVarId->second;
										}
										else
										{
											OutArithmeticCodeSemanticInfo.rightVarId = itVarId->second;
										}
									}
									else
									{
										OutArithmeticCodeSemanticInfo.resultToVarId = itVarId->second;
									}

									isSeccussed = true;
								}
							}
						}

						if ( !isSeccussed )
						{
							result = false;
							break;
						}

					}
					else if ( semanticItem.type == TT_Operator )
					{
						switch ( semanticItem.subType )
						{
						case STT_Appropriate:
							isAref = true;
							break;

						case STT_Add:
							isRightVar = true;
							break;

						default:
							assert( false );
							break;
						}
					}
					else if ( semanticItem.type == TT_Literal )
					{
						OutArithmeticCodeSemanticInfo.isRightConstVar = true;
						EScriptVarType      varType = SubtokenTypeToScriptVarType( tokens[ currentToken ].subType );
						assert( varType != SVT_None );

						std::shared_ptr<FScriptVar>   scriptVar = std::make_shared<FScriptVar>();
						switch ( varType )
						{
						case SVT_String:
							scriptVar->SetString( tokens[ currentToken ].originalView );
							break;

						case SVT_Int:
							scriptVar->SetInt( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;

						case SVT_Bool:
							scriptVar->SetBool( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;
						}

						RegisterVarConstant( scriptVar, &OutArithmeticCodeSemanticInfo.rightVarId );
					}

					++indexSemantic;
				}
				else
				{
					result = false;
					break;
				}
			}

			if ( result )
			{
				InOutTokenIndex = currentToken;
				break;
			}
		}

		return result;
	}

	bool IsSubstructCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FArithmeticCodeSemanticInfo& OutArithmeticCodeSemanticInfo )
	{
		static std::vector< std::vector<FSemanticItem> >       semantics =
		{
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Substruct ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Substruct ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_Substruct ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_Substruct ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     startToken = InOutTokenIndex;
		int     endToken = startToken + InNumTokens;
		int     currentToken = startToken;
		bool    result = true;
		bool	isAref = false;
		for ( int indexSemantics = 0; indexSemantics < semantics.size(); ++indexSemantics )
		{
			bool                                    isRightVar = false;
			const std::vector<FSemanticItem>& semantic = semantics[ indexSemantics ];
			result = true;

			currentToken = startToken;
			for ( int indexSemantic = 0; currentToken <= endToken && indexSemantic < semantic.size(); ++currentToken )
			{
				const FSemanticItem& semanticItem = semantic[ indexSemantic ];
				if ( semanticItem.Match( tokens[ currentToken ] ) )
				{
					if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
					{
						bool    isSeccussed = false;
						{
							auto    itArgVarId = InArgVarsNameToID.find( tokens[ currentToken ].originalView );
							if ( itArgVarId != InArgVarsNameToID.end() )
							{
								if ( isAref )
								{
									if ( !isRightVar )
									{
										OutArithmeticCodeSemanticInfo.leftVarId = itArgVarId->second;
									}
									else
									{
										OutArithmeticCodeSemanticInfo.rightVarId = itArgVarId->second;
									}
								}
								else
								{
									OutArithmeticCodeSemanticInfo.resultToVarId = itArgVarId->second;
								}

								isSeccussed = true;
							}
							else
							{
								auto    itVarId = InVarsNameToID.find( tokens[ currentToken ].originalView );
								if ( itVarId != InVarsNameToID.end() )
								{
									if ( isAref )
									{
										if ( !isRightVar )
										{
											OutArithmeticCodeSemanticInfo.leftVarId = itVarId->second;
										}
										else
										{
											OutArithmeticCodeSemanticInfo.rightVarId = itVarId->second;
										}
									}
									else
									{
										OutArithmeticCodeSemanticInfo.resultToVarId = itVarId->second;
									}

									isSeccussed = true;
								}
							}
						}

						if ( !isSeccussed )
						{
							result = false;
							break;
						}

					}
					else if ( semanticItem.type == TT_Operator )
					{
						switch ( semanticItem.subType )
						{
						case STT_Appropriate:
							isAref = true;
							break;

						case STT_Substruct:
							isRightVar = true;
							break;

						default:
							assert( false );
							break;
						}
					}
					else if ( semanticItem.type == TT_Literal )
					{
						OutArithmeticCodeSemanticInfo.isRightConstVar = true;
						EScriptVarType      varType = SubtokenTypeToScriptVarType( tokens[ currentToken ].subType );
						assert( varType != SVT_None );

						std::shared_ptr<FScriptVar>   scriptVar = std::make_shared<FScriptVar>();
						switch ( varType )
						{
						case SVT_String:
							scriptVar->SetString( tokens[ currentToken ].originalView );
							break;

						case SVT_Int:
							scriptVar->SetInt( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;

						case SVT_Bool:
							scriptVar->SetBool( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;
						}

						RegisterVarConstant( scriptVar, &OutArithmeticCodeSemanticInfo.rightVarId );
					}

					++indexSemantic;
				}
				else
				{
					result = false;
					break;
				}
			}

			if ( result )
			{
				InOutTokenIndex = currentToken;
				break;
			}
		}

		return result;
	}

	bool IsMultiplyCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FArithmeticCodeSemanticInfo& OutArithmeticCodeSemanticInfo )
	{
		static std::vector< std::vector<FSemanticItem> >       semantics =
		{
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Multiply ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Multiply ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_Multiply ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_Multiply ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     startToken = InOutTokenIndex;
		int     endToken = startToken + InNumTokens;
		int     currentToken = startToken;
		bool    result = true;
		bool	isAref = false;
		for ( int indexSemantics = 0; indexSemantics < semantics.size(); ++indexSemantics )
		{
			bool                                    isRightVar = false;
			const std::vector<FSemanticItem>& semantic = semantics[ indexSemantics ];
			result = true;

			currentToken = startToken;
			for ( int indexSemantic = 0; currentToken <= endToken && indexSemantic < semantic.size(); ++currentToken )
			{
				const FSemanticItem& semanticItem = semantic[ indexSemantic ];
				if ( semanticItem.Match( tokens[ currentToken ] ) )
				{
					if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
					{
						bool    isSeccussed = false;
						{
							auto    itArgVarId = InArgVarsNameToID.find( tokens[ currentToken ].originalView );
							if ( itArgVarId != InArgVarsNameToID.end() )
							{
								if ( isAref )
								{
									if ( !isRightVar )
									{
										OutArithmeticCodeSemanticInfo.leftVarId = itArgVarId->second;
									}
									else
									{
										OutArithmeticCodeSemanticInfo.rightVarId = itArgVarId->second;
									}
								}
								else
								{
									OutArithmeticCodeSemanticInfo.resultToVarId = itArgVarId->second;
								}

								isSeccussed = true;
							}
							else
							{
								auto    itVarId = InVarsNameToID.find( tokens[ currentToken ].originalView );
								if ( itVarId != InVarsNameToID.end() )
								{
									if ( isAref )
									{
										if ( !isRightVar )
										{
											OutArithmeticCodeSemanticInfo.leftVarId = itVarId->second;
										}
										else
										{
											OutArithmeticCodeSemanticInfo.rightVarId = itVarId->second;
										}
									}
									else
									{
										OutArithmeticCodeSemanticInfo.resultToVarId = itVarId->second;
									}

									isSeccussed = true;
								}
							}
						}

						if ( !isSeccussed )
						{
							result = false;
							break;
						}

					}
					else if ( semanticItem.type == TT_Operator )
					{
						switch ( semanticItem.subType )
						{
						case STT_Appropriate:
							isAref = true;
							break;

						case STT_Multiply:
							isRightVar = true;
							break;

						default:
							assert( false );
							break;
						}
					}
					else if ( semanticItem.type == TT_Literal )
					{
						OutArithmeticCodeSemanticInfo.isRightConstVar = true;
						EScriptVarType      varType = SubtokenTypeToScriptVarType( tokens[ currentToken ].subType );
						assert( varType != SVT_None );

						std::shared_ptr<FScriptVar>   scriptVar = std::make_shared<FScriptVar>();
						switch ( varType )
						{
						case SVT_String:
							scriptVar->SetString( tokens[ currentToken ].originalView );
							break;

						case SVT_Int:
							scriptVar->SetInt( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;

						case SVT_Bool:
							scriptVar->SetBool( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;
						}

						RegisterVarConstant( scriptVar, &OutArithmeticCodeSemanticInfo.rightVarId );
					}

					++indexSemantic;
				}
				else
				{
					result = false;
					break;
				}
			}

			if ( result )
			{
				InOutTokenIndex = currentToken;
				break;
			}
		}

		return result;
	}

	bool IsDivideCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FArithmeticCodeSemanticInfo& OutArithmeticCodeSemanticInfo )
	{
		static std::vector< std::vector<FSemanticItem> >       semantics =
		{
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Divide ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Divide ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_Divide ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_Divide ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     startToken = InOutTokenIndex;
		int     endToken = startToken + InNumTokens;
		int     currentToken = startToken;
		bool    result = true;
		bool	isAref = false;
		for ( int indexSemantics = 0; indexSemantics < semantics.size(); ++indexSemantics )
		{
			bool                                    isRightVar = false;
			const std::vector<FSemanticItem>& semantic = semantics[ indexSemantics ];
			result = true;

			currentToken = startToken;
			for ( int indexSemantic = 0; currentToken <= endToken && indexSemantic < semantic.size(); ++currentToken )
			{
				const FSemanticItem& semanticItem = semantic[ indexSemantic ];
				if ( semanticItem.Match( tokens[ currentToken ] ) )
				{
					if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
					{
						bool    isSeccussed = false;
						{
							auto    itArgVarId = InArgVarsNameToID.find( tokens[ currentToken ].originalView );
							if ( itArgVarId != InArgVarsNameToID.end() )
							{
								if ( isAref )
								{
									if ( !isRightVar )
									{
										OutArithmeticCodeSemanticInfo.leftVarId = itArgVarId->second;
									}
									else
									{
										OutArithmeticCodeSemanticInfo.rightVarId = itArgVarId->second;
									}
								}
								else
								{
									OutArithmeticCodeSemanticInfo.resultToVarId = itArgVarId->second;
								}

								isSeccussed = true;
							}
							else
							{
								auto    itVarId = InVarsNameToID.find( tokens[ currentToken ].originalView );
								if ( itVarId != InVarsNameToID.end() )
								{
									if ( isAref )
									{
										if ( !isRightVar )
										{
											OutArithmeticCodeSemanticInfo.leftVarId = itVarId->second;
										}
										else
										{
											OutArithmeticCodeSemanticInfo.rightVarId = itVarId->second;
										}
									}
									else
									{
										OutArithmeticCodeSemanticInfo.resultToVarId = itVarId->second;
									}

									isSeccussed = true;
								}
							}
						}

						if ( !isSeccussed )
						{
							result = false;
							break;
						}

					}
					else if ( semanticItem.type == TT_Operator )
					{
						switch ( semanticItem.subType )
						{
						case STT_Appropriate:
							isAref = true;
							break;

						case STT_Divide:
							isRightVar = true;
							break;

						default:
							assert( false );
							break;
						}
					}
					else if ( semanticItem.type == TT_Literal )
					{
						OutArithmeticCodeSemanticInfo.isRightConstVar = true;
						EScriptVarType      varType = SubtokenTypeToScriptVarType( tokens[ currentToken ].subType );
						assert( varType != SVT_None );

						std::shared_ptr<FScriptVar>   scriptVar = std::make_shared<FScriptVar>();
						switch ( varType )
						{
						case SVT_String:
							scriptVar->SetString( tokens[ currentToken ].originalView );
							break;

						case SVT_Int:
							scriptVar->SetInt( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;

						case SVT_Bool:
							scriptVar->SetBool( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;
						}

						RegisterVarConstant( scriptVar, &OutArithmeticCodeSemanticInfo.rightVarId );
					}

					++indexSemantic;
				}
				else
				{
					result = false;
					break;
				}
			}

			if ( result )
			{
				InOutTokenIndex = currentToken;
				break;
			}
		}

		return result;
	}

	bool IsNotCompareCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FCompareSemanticInfo& OutCompareCodeSemanticInfo )
	{
		memset( &OutCompareCodeSemanticInfo, 0, sizeof( OutCompareCodeSemanticInfo ) );
		static std::vector< std::vector<FSemanticItem> >       semantics =
		{
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Not ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Not ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true )
			},
			{
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_Not ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true )
			}
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     startToken = InOutTokenIndex;
		int     endToken = startToken + InNumTokens;
		int     currentToken = startToken;
		bool    result = true;
		for ( int indexSemantics = 0; indexSemantics < semantics.size(); ++indexSemantics )
		{
			bool         isRightVar = false;
			const std::vector<FSemanticItem>& semantic = semantics[ indexSemantics ];
			result = true;

			currentToken = startToken;
			for ( int indexSemantic = 0; currentToken <= endToken && indexSemantic < semantic.size(); ++currentToken )
			{
				const FSemanticItem& semanticItem = semantic[ indexSemantic ];
				if ( semanticItem.Match( tokens[ currentToken ] ) )
				{
					if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
					{
						bool    isSeccussed = false;
						{
							auto    itArgVarId = InArgVarsNameToID.find( tokens[ currentToken ].originalView );
							if ( itArgVarId != InArgVarsNameToID.end() )
							{
								if ( !isRightVar )
								{
									OutCompareCodeSemanticInfo.leftVarId = itArgVarId->second;
								}
								else
								{
									OutCompareCodeSemanticInfo.rightVarId = itArgVarId->second;
								}

								isSeccussed = true;
							}
							else
							{
								auto    itVarId = InVarsNameToID.find( tokens[ currentToken ].originalView );
								if ( itVarId != InVarsNameToID.end() )
								{
									if ( !isRightVar )
									{
										OutCompareCodeSemanticInfo.leftVarId = itVarId->second;
									}
									else
									{
										OutCompareCodeSemanticInfo.rightVarId = itVarId->second;
									}

									isSeccussed = true;
								}
							}
						}

						isRightVar = !isRightVar;

						if ( !isSeccussed )
						{
							result = false;
							break;
						}

					}
					else if ( semanticItem.type == TT_Literal )
					{
						if ( !isRightVar )
						{
							OutCompareCodeSemanticInfo.isLeftConstVar = true;
						}
						else
						{
							OutCompareCodeSemanticInfo.isRightConstVar = true;
						}

						isRightVar = !isRightVar;

						EScriptVarType      varType = SubtokenTypeToScriptVarType( tokens[ currentToken ].subType );
						assert( varType != SVT_None );

						std::shared_ptr<FScriptVar>   scriptVar = std::make_shared<FScriptVar>();
						switch ( varType )
						{
						case SVT_String:
							scriptVar->SetString( tokens[ currentToken ].originalView );
							break;

						case SVT_Int:
							scriptVar->SetInt( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;

						case SVT_Bool:
							scriptVar->SetBool( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;
						}

						RegisterVarConstant( scriptVar, &OutCompareCodeSemanticInfo.rightVarId );
					}

					++indexSemantic;
				}
				else
				{
					result = false;
					break;
				}
			}

			if ( result )
			{
				break;
			}
		}

		InOutTokenIndex = currentToken;
		return result;
	}

	bool IsCompareCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FCompareSemanticInfo& OutCompareCodeSemanticInfo )
	{
		static std::vector< std::vector<FSemanticItem> >       semantics =
		{
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true )
			},
			{
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true )
			}
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     startToken = InOutTokenIndex;
		int     endToken = startToken + InNumTokens;
		int     currentToken = startToken;
		bool    result = true;
		for ( int indexSemantics = 0; indexSemantics < semantics.size(); ++indexSemantics )
		{
			bool         isRightVar = false;
			int         numAppropriate = 0;
			const std::vector<FSemanticItem>& semantic = semantics[ indexSemantics ];
			result = true;

			currentToken = startToken;
			for ( int indexSemantic = 0; currentToken <= endToken && indexSemantic < semantic.size(); ++currentToken )
			{
				const FSemanticItem& semanticItem = semantic[ indexSemantic ];
				if ( semanticItem.Match( tokens[ currentToken ] ) )
				{
					if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
					{
						bool    isSeccussed = false;
						{
							auto    itArgVarId = InArgVarsNameToID.find( tokens[ currentToken ].originalView );
							if ( itArgVarId != InArgVarsNameToID.end() )
							{
								if ( !isRightVar )
								{
									OutCompareCodeSemanticInfo.leftVarId = itArgVarId->second;
								}
								else
								{
									OutCompareCodeSemanticInfo.rightVarId = itArgVarId->second;
								}

								isSeccussed = true;
							}
							else
							{
								auto    itVarId = InVarsNameToID.find( tokens[ currentToken ].originalView );
								if ( itVarId != InVarsNameToID.end() )
								{
									if ( !isRightVar )
									{
										OutCompareCodeSemanticInfo.leftVarId = itVarId->second;
									}
									else
									{
										OutCompareCodeSemanticInfo.rightVarId = itVarId->second;
									}

									isSeccussed = true;
								}
							}
						}

						if ( !isSeccussed )
						{
							result = false;
							break;
						}

					}
					else if ( semanticItem.type == TT_Operator && semanticItem.subType == STT_Appropriate )
					{
						++numAppropriate;
						isRightVar = numAppropriate == 2;
						assert( numAppropriate <= 2 );
					}
					else if ( semanticItem.type == TT_Literal )
					{
						if ( !isRightVar )
						{
							OutCompareCodeSemanticInfo.isLeftConstVar = true;
						}
						else
						{
							OutCompareCodeSemanticInfo.isRightConstVar = true;
						}

						EScriptVarType      varType = SubtokenTypeToScriptVarType( tokens[ currentToken ].subType );
						assert( varType != SVT_None );

						std::shared_ptr<FScriptVar>   scriptVar = std::make_shared<FScriptVar>();
						switch ( varType )
						{
						case SVT_String:
							scriptVar->SetString( tokens[ currentToken ].originalView );
							break;

						case SVT_Int:
							scriptVar->SetInt( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;

						case SVT_Bool:
							scriptVar->SetBool( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;
						}

						RegisterVarConstant( scriptVar, &OutCompareCodeSemanticInfo.rightVarId );
					}

					++indexSemantic;
				}
				else
				{
					result = false;
					break;
				}
			}

			if ( result )
			{
				break;
			}
		}

		InOutTokenIndex = currentToken;
		return result;
	}

	bool IsMoreCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FCompareSemanticInfo& OutCompareCodeSemanticInfo )
	{
		static std::vector< std::vector<FSemanticItem> >       semantics =
		{
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_More ),
				FSemanticItem( TT_Identifier, STT_User )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_More ),
				FSemanticItem( TT_Literal, STT_None, 0, true )
			},
			{
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_More ),
				FSemanticItem( TT_Literal, STT_None, 0, true )
			}
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     startToken = InOutTokenIndex;
		int     endToken = startToken + InNumTokens;
		int     currentToken = startToken;
		bool    result = true;
		for ( int indexSemantics = 0; indexSemantics < semantics.size(); ++indexSemantics )
		{
			bool         isRightVar = false;
			const std::vector<FSemanticItem>& semantic = semantics[ indexSemantics ];
			result = true;

			currentToken = startToken;
			for ( int indexSemantic = 0; currentToken <= endToken && indexSemantic < semantic.size(); ++currentToken )
			{
				const FSemanticItem& semanticItem = semantic[ indexSemantic ];
				if ( semanticItem.Match( tokens[ currentToken ] ) )
				{
					if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
					{
						bool    isSeccussed = false;
						{
							auto    itArgVarId = InArgVarsNameToID.find( tokens[ currentToken ].originalView );
							if ( itArgVarId != InArgVarsNameToID.end() )
							{
								if ( !isRightVar )
								{
									OutCompareCodeSemanticInfo.leftVarId = itArgVarId->second;
								}
								else
								{
									OutCompareCodeSemanticInfo.rightVarId = itArgVarId->second;
								}

								isSeccussed = true;
							}
							else
							{
								auto    itVarId = InVarsNameToID.find( tokens[ currentToken ].originalView );
								if ( itVarId != InVarsNameToID.end() )
								{
									if ( !isRightVar )
									{
										OutCompareCodeSemanticInfo.leftVarId = itVarId->second;
									}
									else
									{
										OutCompareCodeSemanticInfo.rightVarId = itVarId->second;
									}

									isSeccussed = true;
								}
							}
						}

						isRightVar = !isRightVar;

						if ( !isSeccussed )
						{
							result = false;
							break;
						}

					}
					else if ( semanticItem.type == TT_Literal )
					{
						if ( !isRightVar )
						{
							OutCompareCodeSemanticInfo.isLeftConstVar = true;
						}
						else
						{
							OutCompareCodeSemanticInfo.isRightConstVar = true;
						}

						isRightVar = !isRightVar;

						EScriptVarType      varType = SubtokenTypeToScriptVarType( tokens[ currentToken ].subType );
						assert( varType != SVT_None );

						std::shared_ptr<FScriptVar>   scriptVar = std::make_shared<FScriptVar>();
						switch ( varType )
						{
						case SVT_String:
							scriptVar->SetString( tokens[ currentToken ].originalView );
							break;

						case SVT_Int:
							scriptVar->SetInt( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;

						case SVT_Bool:
							scriptVar->SetBool( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;
						}

						RegisterVarConstant( scriptVar, &OutCompareCodeSemanticInfo.rightVarId );
					}

					++indexSemantic;
				}
				else
				{
					result = false;
					break;
				}
			}

			if ( result )
			{
				break;
			}
		}

		InOutTokenIndex = currentToken;
		return result;
	}

	bool IsLessCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FCompareSemanticInfo& OutCompareCodeSemanticInfo )
	{
		static std::vector< std::vector<FSemanticItem> >       semantics =
		{
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Less ),
				FSemanticItem( TT_Identifier, STT_User )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Less ),
				FSemanticItem( TT_Literal, STT_None, 0, true )
			},
			{
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_Less ),
				FSemanticItem( TT_Literal, STT_None, 0, true )
			}
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     startToken = InOutTokenIndex;
		int     endToken = startToken + InNumTokens;
		int     currentToken = startToken;
		bool    result = true;
		for ( int indexSemantics = 0; indexSemantics < semantics.size(); ++indexSemantics )
		{
			bool         isRightVar = false;
			const std::vector<FSemanticItem>& semantic = semantics[ indexSemantics ];
			result = true;

			currentToken = startToken;
			for ( int indexSemantic = 0; currentToken <= endToken && indexSemantic < semantic.size(); ++currentToken )
			{
				const FSemanticItem& semanticItem = semantic[ indexSemantic ];
				if ( semanticItem.Match( tokens[ currentToken ] ) )
				{
					if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
					{
						bool    isSeccussed = false;
						{
							auto    itArgVarId = InArgVarsNameToID.find( tokens[ currentToken ].originalView );
							if ( itArgVarId != InArgVarsNameToID.end() )
							{
								if ( !isRightVar )
								{
									OutCompareCodeSemanticInfo.leftVarId = itArgVarId->second;
								}
								else
								{
									OutCompareCodeSemanticInfo.rightVarId = itArgVarId->second;
								}

								isSeccussed = true;
							}
							else
							{
								auto    itVarId = InVarsNameToID.find( tokens[ currentToken ].originalView );
								if ( itVarId != InVarsNameToID.end() )
								{
									if ( !isRightVar )
									{
										OutCompareCodeSemanticInfo.leftVarId = itVarId->second;
									}
									else
									{
										OutCompareCodeSemanticInfo.rightVarId = itVarId->second;
									}

									isSeccussed = true;
								}
							}
						}

						isRightVar = !isRightVar;

						if ( !isSeccussed )
						{
							result = false;
							break;
						}

					}
					else if ( semanticItem.type == TT_Literal )
					{
						if ( !isRightVar )
						{
							OutCompareCodeSemanticInfo.isLeftConstVar = true;
						}
						else
						{
							OutCompareCodeSemanticInfo.isRightConstVar = true;
						}

						isRightVar = !isRightVar;

						EScriptVarType      varType = SubtokenTypeToScriptVarType( tokens[ currentToken ].subType );
						assert( varType != SVT_None );

						std::shared_ptr<FScriptVar>   scriptVar = std::make_shared<FScriptVar>();
						switch ( varType )
						{
						case SVT_String:
							scriptVar->SetString( tokens[ currentToken ].originalView );
							break;

						case SVT_Int:
							scriptVar->SetInt( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;

						case SVT_Bool:
							scriptVar->SetBool( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;
						}

						RegisterVarConstant( scriptVar, &OutCompareCodeSemanticInfo.rightVarId );
					}

					++indexSemantic;
				}
				else
				{
					result = false;
					break;
				}
			}

			if ( result )
			{
				break;
			}
		}

		InOutTokenIndex = currentToken;
		return result;
	}

	bool IsMoreThenCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FCompareSemanticInfo& OutCompareCodeSemanticInfo )
	{
		static std::vector< std::vector<FSemanticItem> >       semantics =
		{
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_More ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_More ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true )
			},
			{
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_More ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true )
			}
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     startToken = InOutTokenIndex;
		int     endToken = startToken + InNumTokens;
		int     currentToken = startToken;
		bool    result = true;
		for ( int indexSemantics = 0; indexSemantics < semantics.size(); ++indexSemantics )
		{
			bool         isRightVar = false;
			const std::vector<FSemanticItem>& semantic = semantics[ indexSemantics ];
			result = true;

			currentToken = startToken;
			for ( int indexSemantic = 0; currentToken <= endToken && indexSemantic < semantic.size(); ++currentToken )
			{
				const FSemanticItem& semanticItem = semantic[ indexSemantic ];
				if ( semanticItem.Match( tokens[ currentToken ] ) )
				{
					if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
					{
						bool    isSeccussed = false;
						{
							auto    itArgVarId = InArgVarsNameToID.find( tokens[ currentToken ].originalView );
							if ( itArgVarId != InArgVarsNameToID.end() )
							{
								if ( !isRightVar )
								{
									OutCompareCodeSemanticInfo.leftVarId = itArgVarId->second;
								}
								else
								{
									OutCompareCodeSemanticInfo.rightVarId = itArgVarId->second;
								}

								isSeccussed = true;
							}
							else
							{
								auto    itVarId = InVarsNameToID.find( tokens[ currentToken ].originalView );
								if ( itVarId != InVarsNameToID.end() )
								{
									if ( !isRightVar )
									{
										OutCompareCodeSemanticInfo.leftVarId = itVarId->second;
									}
									else
									{
										OutCompareCodeSemanticInfo.rightVarId = itVarId->second;
									}

									isSeccussed = true;
								}
							}
						}

						isRightVar = !isRightVar;

						if ( !isSeccussed )
						{
							result = false;
							break;
						}

					}
					else if ( semanticItem.type == TT_Literal )
					{
						if ( !isRightVar )
						{
							OutCompareCodeSemanticInfo.isLeftConstVar = true;
						}
						else
						{
							OutCompareCodeSemanticInfo.isRightConstVar = true;
						}

						isRightVar = !isRightVar;

						EScriptVarType      varType = SubtokenTypeToScriptVarType( tokens[ currentToken ].subType );
						assert( varType != SVT_None );

						std::shared_ptr<FScriptVar>   scriptVar = std::make_shared<FScriptVar>();
						switch ( varType )
						{
						case SVT_String:
							scriptVar->SetString( tokens[ currentToken ].originalView );
							break;

						case SVT_Int:
							scriptVar->SetInt( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;

						case SVT_Bool:
							scriptVar->SetBool( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;
						}

						RegisterVarConstant( scriptVar, &OutCompareCodeSemanticInfo.rightVarId );
					}

					++indexSemantic;
				}
				else
				{
					result = false;
					break;
				}
			}

			if ( result )
			{
				break;
			}
		}

		InOutTokenIndex = currentToken;
		return result;
	}

	bool IsLessThenCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FCompareSemanticInfo& OutCompareCodeSemanticInfo )
	{
		static std::vector< std::vector<FSemanticItem> >       semantics =
		{
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Less ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Less ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true )
			},
			{
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Operator, STT_Less ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true )
			}
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     startToken = InOutTokenIndex;
		int     endToken = startToken + InNumTokens;
		int     currentToken = startToken;
		bool    result = true;
		for ( int indexSemantics = 0; indexSemantics < semantics.size(); ++indexSemantics )
		{
			bool         isRightVar = false;
			const std::vector<FSemanticItem>& semantic = semantics[ indexSemantics ];
			result = true;

			currentToken = startToken;
			for ( int indexSemantic = 0; currentToken <= endToken && indexSemantic < semantic.size(); ++currentToken )
			{
				const FSemanticItem& semanticItem = semantic[ indexSemantic ];
				if ( semanticItem.Match( tokens[ currentToken ] ) )
				{
					if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
					{
						bool    isSeccussed = false;
						{
							auto    itArgVarId = InArgVarsNameToID.find( tokens[ currentToken ].originalView );
							if ( itArgVarId != InArgVarsNameToID.end() )
							{
								if ( !isRightVar )
								{
									OutCompareCodeSemanticInfo.leftVarId = itArgVarId->second;
								}
								else
								{
									OutCompareCodeSemanticInfo.rightVarId = itArgVarId->second;
								}

								isSeccussed = true;
							}
							else
							{
								auto    itVarId = InVarsNameToID.find( tokens[ currentToken ].originalView );
								if ( itVarId != InVarsNameToID.end() )
								{
									if ( !isRightVar )
									{
										OutCompareCodeSemanticInfo.leftVarId = itVarId->second;
									}
									else
									{
										OutCompareCodeSemanticInfo.rightVarId = itVarId->second;
									}

									isSeccussed = true;
								}
							}
						}

						isRightVar = !isRightVar;

						if ( !isSeccussed )
						{
							result = false;
							break;
						}

					}
					else if ( semanticItem.type == TT_Literal )
					{
						if ( !isRightVar )
						{
							OutCompareCodeSemanticInfo.isLeftConstVar = true;
						}
						else
						{
							OutCompareCodeSemanticInfo.isRightConstVar = true;
						}

						isRightVar = !isRightVar;

						EScriptVarType      varType = SubtokenTypeToScriptVarType( tokens[ currentToken ].subType );
						assert( varType != SVT_None );

						std::shared_ptr<FScriptVar>   scriptVar = std::make_shared<FScriptVar>();
						switch ( varType )
						{
						case SVT_String:
							scriptVar->SetString( tokens[ currentToken ].originalView );
							break;

						case SVT_Int:
							scriptVar->SetInt( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;

						case SVT_Bool:
							scriptVar->SetBool( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;
						}

						RegisterVarConstant( scriptVar, &OutCompareCodeSemanticInfo.rightVarId );
					}

					++indexSemantic;
				}
				else
				{
					result = false;
					break;
				}
			}

			if ( result )
			{
				break;
			}
		}

		InOutTokenIndex = currentToken;
		return result;
	}

	bool IsWhileCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FWhileCodeSemanticInfo& OutWhileCodeSemanticInfo )
	{
		memset( &OutWhileCodeSemanticInfo, 0, sizeof( OutWhileCodeSemanticInfo ) );
		static std::vector<FSemanticItem>       semantics =
		{
			FSemanticItem( TT_Keyword, STT_While ),
			FSemanticItem( TT_Delimeter, STT_BeginArgs, -1 ),
			FSemanticItem( TT_Delimeter, STT_EndArgs ),
			FSemanticItem( TT_Delimeter, STT_BeginBody, -1 ),
			FSemanticItem( TT_Delimeter, STT_EndBody )
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int		bodyLevel = -1;
		int     numPlacableTokens = 0;
		for ( int index = 0; InOutTokenIndex < tokens.size() && index < semantics.size(); ++InOutTokenIndex )
		{
			const FSemanticItem& semanticItem = semantics[ index ];
			if ( semanticItem.Match( tokens[ InOutTokenIndex ] ) || bodyLevel > -1 )
			{
				if ( bodyLevel == -1 )
				{
					if ( semanticItem.subType == STT_BeginArgs )
					{
						OutWhileCodeSemanticInfo.startIfArgs = InOutTokenIndex + 1;
					}
					else if ( semanticItem.subType == STT_EndArgs )
					{
						OutWhileCodeSemanticInfo.numIfArgs = InOutTokenIndex - OutWhileCodeSemanticInfo.startIfArgs;
					}
					else if ( semanticItem.subType == STT_BeginBody )
					{
						OutWhileCodeSemanticInfo.startBody = InOutTokenIndex + 1;
					}
					/*else if ( semanticItem.subType == STT_EndBody )
					{
						OutWhileCodeSemanticInfo.numBody = InOutTokenIndex - OutWhileCodeSemanticInfo.startBody;
					}*/
				}

				if ( tokens[ InOutTokenIndex ].subType == STT_BeginBody )
				{
					++bodyLevel;
				}
				else if ( tokens[ InOutTokenIndex ].subType == STT_EndBody )
				{
					if ( bodyLevel - 1 == -1 )
					{
						OutWhileCodeSemanticInfo.numBody = InOutTokenIndex - OutWhileCodeSemanticInfo.startBody;
					}
					--bodyLevel;
				}

				numPlacableTokens = semanticItem.numPlacableTokens;
				if ( bodyLevel == -1 || semanticItem.subType == STT_BeginBody )
				{
					++index;
				}
			}
			else if ( numPlacableTokens > 0 || numPlacableTokens == -1 )
			{
				if ( numPlacableTokens > 0 )
				{
					--numPlacableTokens;
				}
				continue;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	bool IsIfCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FIfCodeSemanticInfo& OutIfCodeSemanticInfo )
	{
		memset( &OutIfCodeSemanticInfo, 0, sizeof( OutIfCodeSemanticInfo ) );
		static std::vector<FSemanticItem>       semantics =
		{
			FSemanticItem( TT_Keyword, STT_If ),
			FSemanticItem( TT_Delimeter, STT_BeginArgs, -1 ),
			FSemanticItem( TT_Delimeter, STT_EndArgs ),
			FSemanticItem( TT_Delimeter, STT_BeginBody, -1 ),
			FSemanticItem( TT_Delimeter, STT_EndBody ),
			FSemanticItem( TT_Keyword, STT_Else, 0, false, true ),
			FSemanticItem( TT_Delimeter, STT_BeginBody, -1 ),
			FSemanticItem( TT_Delimeter, STT_EndBody )
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int		bodyLevel = -1;
		bool    isIfBlock = true;       // true - if block, false - else block
		int     numPlacableTokens = 0;
		for ( int index = 0; InOutTokenIndex < tokens.size() && index < semantics.size(); ++InOutTokenIndex )
		{
			const FSemanticItem& semanticItem = semantics[ index ];
			if ( semanticItem.Match( tokens[ InOutTokenIndex ] ) || bodyLevel > -1 )
			{
				if ( !semanticItem.Match( tokens[ InOutTokenIndex ], true ) && bodyLevel == -1 )
				{
					break;
				}

				if ( bodyLevel == -1 )
				{
					if ( semanticItem.subType == STT_BeginArgs )
					{
						OutIfCodeSemanticInfo.startIfArgs = InOutTokenIndex + 1;
					}
					else if ( semanticItem.subType == STT_EndArgs )
					{
						OutIfCodeSemanticInfo.numIfArgs = InOutTokenIndex - OutIfCodeSemanticInfo.startIfArgs;
					}
					else if ( semanticItem.subType == STT_BeginBody )
					{
						if ( isIfBlock )
						{
							OutIfCodeSemanticInfo.startBody = InOutTokenIndex + 1;
						}
						else
						{
							OutIfCodeSemanticInfo.startElseBody = InOutTokenIndex + 1;
						}
					}
					/*else if ( semanticItem.subType == STT_EndBody )
					{
						if ( isIfBlock )
						{
							OutIfCodeSemanticInfo.numBody = InOutTokenIndex - OutIfCodeSemanticInfo.startBody;
						}
						else
						{
							OutIfCodeSemanticInfo.numElseBody = InOutTokenIndex - OutIfCodeSemanticInfo.startElseBody;
						}
					}*/
					else if ( semanticItem.type == TT_Keyword )
					{
						switch ( semanticItem.subType )
						{
						case STT_If:
							isIfBlock = true;
							break;

						case STT_Else:
							isIfBlock = false;
							break;
						}
					}
				}

				if ( tokens[ InOutTokenIndex ].subType == STT_BeginBody )
				{
					++bodyLevel;
				}
				else if ( tokens[ InOutTokenIndex ].subType == STT_EndBody )
				{
					if ( bodyLevel - 1 == -1 )
					{
						if ( isIfBlock )
						{
							OutIfCodeSemanticInfo.numBody = InOutTokenIndex - OutIfCodeSemanticInfo.startBody;
						}
						else
						{
							OutIfCodeSemanticInfo.numElseBody = InOutTokenIndex - OutIfCodeSemanticInfo.startElseBody;
						}
					}
					--bodyLevel;
				}

				numPlacableTokens = semanticItem.numPlacableTokens;
				if ( bodyLevel == -1 || semanticItem.subType == STT_BeginBody )
				{
					++index;
				}
			}
			else if ( numPlacableTokens > 0 || numPlacableTokens == -1 )
			{
				if ( numPlacableTokens > 0 )
				{
					--numPlacableTokens;
				}
				continue;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	bool IsAssignCodeSemantic( int& InOutTokenIndex, int InNumTokens, const std::unordered_map<std::string, int>& InVarsNameToID, const std::unordered_map<std::string, int>& InArgVarsNameToID, FAssignCodeSemanticInfo& OutAssignCodeSemanticInfo )
	{
		static std::vector< std::vector<FSemanticItem> >       semantics =
		{
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			},
			{
				FSemanticItem( TT_Identifier, STT_User ),
				FSemanticItem( TT_Operator, STT_Appropriate ),
				FSemanticItem( TT_Literal, STT_None, 0, true ),
				FSemanticItem( TT_Delimeter, STT_EndCodeLine )
			}
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     startToken = InOutTokenIndex;
		int     endToken = startToken + InNumTokens;
		int     currentToken = startToken;
		bool    result = true;
		for ( int indexSemantics = 0; indexSemantics < semantics.size(); ++indexSemantics )
		{
			bool                                    isRightVar = false;
			const std::vector<FSemanticItem>& semantic = semantics[ indexSemantics ];
			result = true;

			currentToken = startToken;
			for ( int indexSemantic = 0; currentToken <= endToken && indexSemantic < semantic.size(); ++currentToken )
			{
				const FSemanticItem& semanticItem = semantic[ indexSemantic ];
				if ( semanticItem.Match( tokens[ currentToken ] ) )
				{
					if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
					{
						bool    isSeccussed = false;
						{
							auto    itArgVarId = InArgVarsNameToID.find( tokens[ currentToken ].originalView );
							if ( itArgVarId != InArgVarsNameToID.end() )
							{
								if ( !isRightVar )
								{
									OutAssignCodeSemanticInfo.leftVarId = itArgVarId->second;
								}
								else
								{
									OutAssignCodeSemanticInfo.rightVarId = itArgVarId->second;
								}

								isSeccussed = true;
							}
							else
							{
								auto    itVarId = InVarsNameToID.find( tokens[ currentToken ].originalView );
								if ( itVarId != InVarsNameToID.end() )
								{
									if ( !isRightVar )
									{
										OutAssignCodeSemanticInfo.leftVarId = itVarId->second;
									}
									else
									{
										OutAssignCodeSemanticInfo.rightVarId = itVarId->second;
									}

									isSeccussed = true;
								}
							}
						}

						if ( !isSeccussed )
						{
							result = false;
							break;
						}

					}
					else if ( semanticItem.type == TT_Operator && semanticItem.subType == STT_Appropriate )
					{
						isRightVar = true;
					}
					else if ( semanticItem.type == TT_Literal )
					{
						OutAssignCodeSemanticInfo.isRightConstVar = true;
						EScriptVarType      varType = SubtokenTypeToScriptVarType( tokens[ currentToken ].subType );
						assert( varType != SVT_None );

						std::shared_ptr<FScriptVar>   scriptVar = std::make_shared<FScriptVar>();
						switch ( varType )
						{
						case SVT_String:
							scriptVar->SetString( tokens[ currentToken ].originalView );
							break;

						case SVT_Int:
							scriptVar->SetInt( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;

						case SVT_Bool:
							scriptVar->SetBool( atoi( tokens[ currentToken ].originalView.c_str() ) );
							break;
						}

						RegisterVarConstant( scriptVar, &OutAssignCodeSemanticInfo.rightVarId );
					}

					++indexSemantic;
				}
				else
				{
					result = false;
					break;
				}
			}

			if ( result )
			{
				InOutTokenIndex = currentToken;
				break;
			}
		}


		return result;
	}

	bool IsDeclVarInArgsSemantic( int& InOutTokenIndex, int InNumTokens, FAllocateVarCodeSemanticInfo& OutAllocateVarCodeSemanticInfo )
	{
		static std::vector<FSemanticItem>       semantics =
		{
			FSemanticItem( TT_Keyword, STT_None, 0, true ),
			FSemanticItem( TT_Identifier, STT_User )
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     endToken = InOutTokenIndex + InNumTokens;
		for ( int index = 0; InOutTokenIndex <= endToken && index < semantics.size(); ++InOutTokenIndex )
		{
			const FSemanticItem& semanticItem = semantics[ index ];
			if ( semanticItem.Match( tokens[ InOutTokenIndex ] ) )
			{
				if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
				{
					OutAllocateVarCodeSemanticInfo.name = tokens[ InOutTokenIndex ].originalView;
				}
				else if ( semanticItem.type == TT_Keyword )
				{
					OutAllocateVarCodeSemanticInfo.varType = SubtokenTypeToScriptVarType( tokens[ InOutTokenIndex ].subType );
					assert( OutAllocateVarCodeSemanticInfo.varType != SVT_None );
				}

				++index;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	bool IsAllocateVarCodeSemantic( int& InOutTokenIndex, int InNumTokens, FAllocateVarCodeSemanticInfo& OutAllocateVarCodeSemanticInfo )
	{
		static std::vector<FSemanticItem>       semantics =
		{
			FSemanticItem( TT_Keyword, STT_None, 0, true ),
			FSemanticItem( TT_Identifier, STT_User ),
			FSemanticItem( TT_Delimeter, STT_EndCodeLine )
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int		oldToken = InOutTokenIndex;;
		int     endToken = InOutTokenIndex + InNumTokens;
		for ( int index = 0; InOutTokenIndex <= endToken && index < semantics.size(); ++InOutTokenIndex )
		{
			const FSemanticItem& semanticItem = semantics[ index ];
			if ( semanticItem.Match( tokens[ InOutTokenIndex ] ) )
			{
				if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
				{
					OutAllocateVarCodeSemanticInfo.name = tokens[ InOutTokenIndex ].originalView;
				}
				else if ( semanticItem.type == TT_Keyword )
				{
					OutAllocateVarCodeSemanticInfo.varType = SubtokenTypeToScriptVarType( tokens[ InOutTokenIndex ].subType );
					if ( OutAllocateVarCodeSemanticInfo.varType == SVT_None )
					{
						InOutTokenIndex = oldToken;
						return false;
					}
				}

				++index;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	bool IsCallCodeSemantic( int& InOutTokenIndex, int InNumTokens, FCallCodeSemanticInfo& OutCallCodeSemanticInfo )
	{
		static std::vector<FSemanticItem>       semantics =
		{
			FSemanticItem( TT_Identifier, STT_User ),
			FSemanticItem( TT_Delimeter, STT_BeginArgs, -1 ),
			FSemanticItem( TT_Delimeter, STT_EndArgs ),
			FSemanticItem( TT_Delimeter, STT_EndCodeLine )
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int     endToken = InOutTokenIndex + InNumTokens;
		int     numPlacableTokens = 0;
		for ( int index = 0; InOutTokenIndex <= endToken && index < semantics.size(); ++InOutTokenIndex )
		{
			const FSemanticItem& semanticItem = semantics[ index ];
			if ( semanticItem.Match( tokens[ InOutTokenIndex ] ) )
			{
				if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
				{
					int     funcId;
					bool    isNative = false;
					if ( !GetFunctionInfoByName( tokens[ InOutTokenIndex ].originalView, funcId, isNative ) )
					{
						return false;
					}

					OutCallCodeSemanticInfo.isNativeFunction = isNative;
					OutCallCodeSemanticInfo.functionId = funcId;
				}
				else if ( semanticItem.subType == STT_BeginArgs )
				{
					OutCallCodeSemanticInfo.startArgs = InOutTokenIndex + 1;
				}
				else if ( semanticItem.subType == STT_EndArgs )
				{
					OutCallCodeSemanticInfo.numArgs = InOutTokenIndex - OutCallCodeSemanticInfo.startArgs;
				}

				numPlacableTokens = semanticItem.numPlacableTokens;
				++index;
			}
			else if ( numPlacableTokens > 0 || numPlacableTokens == -1 )
			{
				if ( numPlacableTokens > 0 )
				{
					--numPlacableTokens;
				}
				continue;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	bool IsDeclFunctionSemantic( int& InOutTokenIndex, FDeclFunctionSemanticInfo& OutDeclFunctionSemanticInfo )
	{
		static std::vector<FSemanticItem>  semantics =
		{
			FSemanticItem( TT_Keyword, STT_None ),
			FSemanticItem( TT_Identifier, STT_User ),
			FSemanticItem( TT_Delimeter, STT_BeginArgs, -1 ),
			FSemanticItem( TT_Delimeter, STT_EndArgs ),
			FSemanticItem( TT_Delimeter, STT_BeginBody, -1 ),
			FSemanticItem( TT_Delimeter, STT_EndBody )
		};

		if ( tokens.size() - InOutTokenIndex < semantics.size() )
		{
			return false;
		}

		int		bodyLevel = -1;
		int     numPlacableTokens = 0;
		for ( int index = 0; InOutTokenIndex < tokens.size() && index < semantics.size(); ++InOutTokenIndex )
		{
			const FSemanticItem& semanticItem = semantics[ index ];
			if ( semanticItem.Match( tokens[ InOutTokenIndex ] ) || bodyLevel > -1 )
			{
				if ( bodyLevel == -1 )
				{
					if ( semanticItem.type == TT_Identifier && semanticItem.subType == STT_User )
					{
						OutDeclFunctionSemanticInfo.name = tokens[ InOutTokenIndex ].originalView;
					}
					else if ( semanticItem.subType == STT_BeginArgs )
					{
						OutDeclFunctionSemanticInfo.startArgs = InOutTokenIndex + 1;
					}
					else if ( semanticItem.subType == STT_EndArgs )
					{
						OutDeclFunctionSemanticInfo.numArgs = InOutTokenIndex - OutDeclFunctionSemanticInfo.startArgs;
					}
					else if ( semanticItem.subType == STT_BeginBody )
					{
						OutDeclFunctionSemanticInfo.startBody = InOutTokenIndex + 1;
					}
				}

				if ( tokens[ InOutTokenIndex ].subType == STT_BeginBody )
				{
					++bodyLevel;
				}
				else if ( tokens[ InOutTokenIndex ].subType == STT_EndBody )
				{
					if ( bodyLevel - 1 == -1 )
					{
						OutDeclFunctionSemanticInfo.numBody = InOutTokenIndex - OutDeclFunctionSemanticInfo.startBody;
					}
					--bodyLevel;
				}

				numPlacableTokens = semanticItem.numPlacableTokens;
				if ( bodyLevel == -1 || semanticItem.subType == STT_BeginBody )
				{
					++index;
				}
			}
			else if ( numPlacableTokens > 0 || numPlacableTokens == -1 )
			{
				if ( numPlacableTokens > 0 )
				{
					--numPlacableTokens;
				}
				continue;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	void RegisterFunction( const FFunction& InFunction )
	{
		int     functionId = functions.size();
		functions.push_back( InFunction );
		functionNameToID[ InFunction.GetName() ] = functionId;
	}

	void RegisterNativeFunction( const std::string& InFuncName, FNativeFunctionFn InFn )
	{
		int     nativeFunctionId = nativeFunctions.size();
		nativeFunctions.push_back( FNativeFunction{ InFuncName, InFn } );
		nativeFunctionNameToID[ InFuncName ] = nativeFunctionId;
	}

	void RegisterVarConstant( std::shared_ptr<FScriptVar>& InVar, int* InVarId = nullptr )
	{
		if ( InVarId )
		{
			*InVarId = varConstants.size();
		}
		varConstants.push_back( InVar );
	}

	bool GetFunctionInfoByName( const std::string& InFuncName, int& InFuncId, bool& InIsNativeFunc )
	{
		auto    itFunc = functionNameToID.find( InFuncName );
		if ( itFunc != functionNameToID.end() )
		{
			InFuncId = itFunc->second;
			InIsNativeFunc = false;
			return true;
		}

		auto    itNativeFunc = nativeFunctionNameToID.find( InFuncName );
		if ( itNativeFunc != nativeFunctionNameToID.end() )
		{
			InFuncId = itNativeFunc->second;
			InIsNativeFunc = true;
			return true;
		}

		return false;
	}

	std::vector< FToken >                                                                            tokens;             // Array of tokens
	std::unordered_map< FToken, unsigned int, FToken::FTokenKeyFunc, FToken::FTokenEqualFunc >       userIdentifiers;    // User identifiers map

	std::unordered_map<std::string, int>          functionNameToID;         // Function name to id
	std::unordered_map<std::string, int>          nativeFunctionNameToID;   // Native function name to id
	std::vector<FFunction>                        functions;                // Functions
	std::vector<FNativeFunction>                  nativeFunctions;          // Native functions
	std::vector<std::shared_ptr<FScriptVar>>      varConstants;             // Var constants
};

/** C translator */
FCTranslator        GCTranslator;

void FFunction::Execute( FFrame& InFrame )
{
	bool        isCompareResult = false;

	std::shared_ptr<FScriptVar>		registers[ SR_Num ] =
	{
		std::make_shared<FScriptVar>()
	};

	for ( int i = 0; i < code.size(); )
	{
		EScriptOperation        op = ( EScriptOperation ) code[ i ];
		switch ( op )
		{
		case Op_Call:
		{
			int              functionId = code[ i + 1 ];
			int              numArgs = code[ i + 2 ];
			FFrame           callFrame;

			for ( int j = 0; j < numArgs; ++j )
			{
				int     varFlag = code[ i + 3 + j * 2 ];
				int     varId = code[ i + 4 + j * 2 ];

				switch ( varFlag )
				{
				case SVF_User:
					callFrame.args.push_back( InFrame.vars[ varId ] );
					break;

				case SVF_Const:
					callFrame.args.push_back( GCTranslator.GetVarConstant( varId ) );
					break;

				case SVF_Arg:
					callFrame.args.push_back( InFrame.args[ varId ] );
					break;
				}
			}

			GCTranslator.ExecuteFunction( functionId, false, callFrame );

			i += 3 + ( numArgs * 2 );
			break;
		}

		case Op_NativeCall:
		{
			int              codeOffset = 1;
			int              functionId = code[ i + codeOffset ];
			++codeOffset;
			int              numArgs = code[ i + codeOffset ];
			FFrame           callFrame;

			if ( numArgs > 0 )
			{
				++codeOffset;
			}

			for ( int j = 0; j < numArgs; ++j )
			{
				int     varFlag = code[ i + codeOffset ];
				int     varId = code[ i + codeOffset + 1 ];

				switch ( varFlag )
				{
				case SVF_User:
					callFrame.args.push_back( InFrame.vars[ varId ] );
					break;

				case SVF_Const:
					callFrame.args.push_back( GCTranslator.GetVarConstant( varId ) );
					break;

				case SVF_Arg:
					callFrame.args.push_back( InFrame.args[ varId ] );
					break;
				}

				codeOffset += 2;
			}

			GCTranslator.ExecuteFunction( functionId, true, callFrame );
			i += codeOffset;
			break;
		}

		case  Op_AllocateVar:
		{
			std::shared_ptr<FScriptVar>      scriptVar = std::make_shared<FScriptVar>();
			EScriptVarType  varType = ( EScriptVarType ) code[ i + 1 ];

			switch ( varType )
			{
			case SVT_Int:
				scriptVar->SetInt( 0 );
				break;

			case SVT_String:
				scriptVar->SetString( "" );
				break;

			case SVT_Bool:
				scriptVar->SetBool( false );
				break;
			}

			InFrame.vars.push_back( scriptVar );

			i += 2;
			break;
		}

		case Op_Assign:
		{
			int         leftVarId = code[ i + 1 ];
			int         rightVarFlag = code[ i + 2 ];
			int         rightVarId = code[ i + 3 ];

			switch ( rightVarFlag )
			{
			case SVF_User:
				InFrame.vars[ leftVarId ]->Set( InFrame.vars[ rightVarId ] );
				break;

			case SVF_Const:
				InFrame.vars[ leftVarId ]->Set( GCTranslator.GetVarConstant( rightVarId ) );
				break;

			case SVF_Arg:
				InFrame.vars[ leftVarId ]->Set( InFrame.args[ rightVarId ] );
				break;

			case SVF_Register:
				InFrame.vars[ leftVarId ]->Set( registers[ rightVarId ] );
				break;

			default:
				assert( false );
				break;
			}

			i += 4;
			break;
		}

		case Op_Add:
		{
			int         leftVarId = code[ i + 1 ];
			int         rightVarFlag = code[ i + 2 ];
			int         rightVarId = code[ i + 3 ];

			switch ( rightVarFlag )
			{
			case SVF_User:
				registers[ SR_AX ]->Set( FScriptVar::Add( InFrame.vars[ leftVarId ], InFrame.vars[ rightVarId ] ) );
				break;

			case SVF_Const:
				registers[ SR_AX ]->Set( FScriptVar::Add( InFrame.vars[ leftVarId ], GCTranslator.GetVarConstant( rightVarId ) ) );
				break;

			case SVF_Arg:
				registers[ SR_AX ]->Set( FScriptVar::Add( InFrame.vars[ leftVarId ], InFrame.args[ rightVarId ] ) );
				break;

			default:
				assert( false );
				break;
			}

			i += 4;
			break;
		}

		case Op_Substruct:
		{
			int         leftVarId = code[ i + 1 ];
			int         rightVarFlag = code[ i + 2 ];
			int         rightVarId = code[ i + 3 ];

			switch ( rightVarFlag )
			{
			case SVF_User:
				registers[ SR_AX ]->Set( FScriptVar::Substruct( InFrame.vars[ leftVarId ], InFrame.vars[ rightVarId ] ) );
				break;

			case SVF_Const:
				registers[ SR_AX ]->Set( FScriptVar::Substruct( InFrame.vars[ leftVarId ], GCTranslator.GetVarConstant( rightVarId ) ) );
				break;

			case SVF_Arg:
				registers[ SR_AX ]->Set( FScriptVar::Substruct( InFrame.vars[ leftVarId ], InFrame.args[ rightVarId ] ) );
				break;

			default:
				assert( false );
				break;
			}

			i += 4;
			break;
		}

		case Op_Multiply:
		{
			int         leftVarId = code[ i + 1 ];
			int         rightVarFlag = code[ i + 2 ];
			int         rightVarId = code[ i + 3 ];

			switch ( rightVarFlag )
			{
			case SVF_User:
				registers[ SR_AX ]->Set( FScriptVar::Multiply( InFrame.vars[ leftVarId ], InFrame.vars[ rightVarId ] ) );
				break;

			case SVF_Const:
				registers[ SR_AX ]->Set( FScriptVar::Multiply( InFrame.vars[ leftVarId ], GCTranslator.GetVarConstant( rightVarId ) ) );
				break;

			case SVF_Arg:
				registers[ SR_AX ]->Set( FScriptVar::Multiply( InFrame.vars[ leftVarId ], InFrame.args[ rightVarId ] ) );
				break;

			default:
				assert( false );
				break;
			}

			i += 4;
			break;
		}

		case Op_Divide:
		{
			int         leftVarId = code[ i + 1 ];
			int         rightVarFlag = code[ i + 2 ];
			int         rightVarId = code[ i + 3 ];

			switch ( rightVarFlag )
			{
			case SVF_User:
				registers[ SR_AX ]->Set( FScriptVar::Divide( InFrame.vars[ leftVarId ], InFrame.vars[ rightVarId ] ) );
				break;

			case SVF_Const:
				registers[ SR_AX ]->Set( FScriptVar::Divide( InFrame.vars[ leftVarId ], GCTranslator.GetVarConstant( rightVarId ) ) );
				break;

			case SVF_Arg:
				registers[ SR_AX ]->Set( FScriptVar::Divide( InFrame.vars[ leftVarId ], InFrame.args[ rightVarId ] ) );
				break;

			default:
				assert( false );
				break;
			}

			i += 4;
			break;
		}

		case Op_Compare:
		{
			int         leftVarFlag = code[ i + 1 ];
			int         leftVarId = code[ i + 2 ];
			int         rightVarFlag = code[ i + 3 ];
			int         rightVarId = code[ i + 4 ];

			std::shared_ptr<FScriptVar>  leftVar;
			std::shared_ptr<FScriptVar>  rightVar;

			switch ( leftVarFlag )
			{
			case SVF_User:
				leftVar = InFrame.vars[ leftVarId ];
				break;

			case SVF_Const:
				leftVar = GCTranslator.GetVarConstant( leftVarId );
				break;

			case SVF_Arg:
				leftVar = InFrame.args[ leftVarId ];
				break;
			}

			switch ( rightVarFlag )
			{
			case SVF_User:
				rightVar = InFrame.vars[ rightVarId ];
				break;

			case SVF_Const:
				rightVar = GCTranslator.GetVarConstant( rightVarId );
				break;

			case SVF_Arg:
				rightVar = InFrame.args[ rightVarId ];
				break;
			}

			isCompareResult = leftVar->Compare( rightVar );
			i += 5;
			break;
		}

		case Op_NotCompare:
		{
			int         leftVarFlag = code[ i + 1 ];
			int         leftVarId = code[ i + 2 ];
			int         rightVarFlag = code[ i + 3 ];
			int         rightVarId = code[ i + 4 ];

			std::shared_ptr<FScriptVar>  leftVar;
			std::shared_ptr<FScriptVar>  rightVar;

			switch ( leftVarFlag )
			{
			case SVF_User:
				leftVar = InFrame.vars[ leftVarId ];
				break;

			case SVF_Const:
				leftVar = GCTranslator.GetVarConstant( leftVarId );
				break;

			case SVF_Arg:
				leftVar = InFrame.args[ leftVarId ];
				break;
			}

			switch ( rightVarFlag )
			{
			case SVF_User:
				rightVar = InFrame.vars[ rightVarId ];
				break;

			case SVF_Const:
				rightVar = GCTranslator.GetVarConstant( rightVarId );
				break;

			case SVF_Arg:
				rightVar = InFrame.args[ rightVarId ];
				break;
			}

			isCompareResult = !leftVar->Compare( rightVar );
			i += 5;
			break;
		}

		case Op_More:
		{
			int         leftVarFlag = code[ i + 1 ];
			int         leftVarId = code[ i + 2 ];
			int         rightVarFlag = code[ i + 3 ];
			int         rightVarId = code[ i + 4 ];

			std::shared_ptr<FScriptVar>  leftVar;
			std::shared_ptr<FScriptVar>  rightVar;

			switch ( leftVarFlag )
			{
			case SVF_User:
				leftVar = InFrame.vars[ leftVarId ];
				break;

			case SVF_Const:
				leftVar = GCTranslator.GetVarConstant( leftVarId );
				break;

			case SVF_Arg:
				leftVar = InFrame.args[ leftVarId ];
				break;
			}

			switch ( rightVarFlag )
			{
			case SVF_User:
				rightVar = InFrame.vars[ rightVarId ];
				break;

			case SVF_Const:
				rightVar = GCTranslator.GetVarConstant( rightVarId );
				break;

			case SVF_Arg:
				rightVar = InFrame.args[ rightVarId ];
				break;
			}

			isCompareResult = leftVar->More( rightVar );
			i += 5;
			break;
		}

		case Op_MoreThen:
		{
			int         leftVarFlag = code[ i + 1 ];
			int         leftVarId = code[ i + 2 ];
			int         rightVarFlag = code[ i + 3 ];
			int         rightVarId = code[ i + 4 ];

			std::shared_ptr<FScriptVar>  leftVar;
			std::shared_ptr<FScriptVar>  rightVar;

			switch ( leftVarFlag )
			{
			case SVF_User:
				leftVar = InFrame.vars[ leftVarId ];
				break;

			case SVF_Const:
				leftVar = GCTranslator.GetVarConstant( leftVarId );
				break;

			case SVF_Arg:
				leftVar = InFrame.args[ leftVarId ];
				break;
			}

			switch ( rightVarFlag )
			{
			case SVF_User:
				rightVar = InFrame.vars[ rightVarId ];
				break;

			case SVF_Const:
				rightVar = GCTranslator.GetVarConstant( rightVarId );
				break;

			case SVF_Arg:
				rightVar = InFrame.args[ rightVarId ];
				break;
			}

			isCompareResult = leftVar->MoreThen( rightVar );
			i += 5;
			break;
		}

		case Op_Less:
		{
			int         leftVarFlag = code[ i + 1 ];
			int         leftVarId = code[ i + 2 ];
			int         rightVarFlag = code[ i + 3 ];
			int         rightVarId = code[ i + 4 ];

			std::shared_ptr<FScriptVar>  leftVar;
			std::shared_ptr<FScriptVar>  rightVar;

			switch ( leftVarFlag )
			{
			case SVF_User:
				leftVar = InFrame.vars[ leftVarId ];
				break;

			case SVF_Const:
				leftVar = GCTranslator.GetVarConstant( leftVarId );
				break;

			case SVF_Arg:
				leftVar = InFrame.args[ leftVarId ];
				break;
			}

			switch ( rightVarFlag )
			{
			case SVF_User:
				rightVar = InFrame.vars[ rightVarId ];
				break;

			case SVF_Const:
				rightVar = GCTranslator.GetVarConstant( rightVarId );
				break;

			case SVF_Arg:
				rightVar = InFrame.args[ rightVarId ];
				break;
			}

			isCompareResult = leftVar->Less( rightVar );
			i += 5;
			break;
		}

		case Op_LessThen:
		{
			int         leftVarFlag = code[ i + 1 ];
			int         leftVarId = code[ i + 2 ];
			int         rightVarFlag = code[ i + 3 ];
			int         rightVarId = code[ i + 4 ];

			std::shared_ptr<FScriptVar>  leftVar;
			std::shared_ptr<FScriptVar>  rightVar;

			switch ( leftVarFlag )
			{
			case SVF_User:
				leftVar = InFrame.vars[ leftVarId ];
				break;

			case SVF_Const:
				leftVar = GCTranslator.GetVarConstant( leftVarId );
				break;

			case SVF_Arg:
				leftVar = InFrame.args[ leftVarId ];
				break;
			}

			switch ( rightVarFlag )
			{
			case SVF_User:
				rightVar = InFrame.vars[ rightVarId ];
				break;

			case SVF_Const:
				rightVar = GCTranslator.GetVarConstant( rightVarId );
				break;

			case SVF_Arg:
				rightVar = InFrame.args[ rightVarId ];
				break;
			}

			isCompareResult = leftVar->LessThen( rightVar );
			i += 5;
			break;
		}

		case Op_JumpNotEqual:
		{
			if ( !isCompareResult )
			{
				i = code[ i + 1 ];
			}
			else
			{
				i += 2;
			}
			break;
		}

		case Op_JumpEqual:
		{
			if ( isCompareResult )
			{
				i = code[ i + 1 ];
			}
			else
			{
				i += 2;
			}
			break;
		}

		case Op_Jump:
		{
			i = code[ i + 1 ];
			break;
		}

		default:
			++i;
			break;
		}
	}
}

enum EMenuSection
{
	MS_None,
	MS_LoadFile,
	MS_ShowTokens,
	MS_ShowUserIdentifiers,
	MS_ShowFunctions,
	MS_CallScriptFunction,
	MS_Exit
};

int main()
{
	GCTranslator.Init();

	int     indexMenu = MS_None;
	while ( indexMenu != MS_Exit )
	{
		system( "cls" );
		printf( "Select menu section:\n"
				"1. Load script code from file\n"
				"2. Show tokens\n"
				"3. Show user identifiers\n"
				"4. Show all functions\n"
				"5. Call script function\n"
				"6. Exit\n\n> " );
		scanf( "%i", &indexMenu );

		switch ( indexMenu )
		{
		case MS_LoadFile:
		{
			std::string     sourceCodePath;

			system( "cls" );
			printf( "-- Load script code --\n"
					"Enter path: " );
			std::cin >> sourceCodePath;

			GCTranslator.LoadFromFile( sourceCodePath );
			system( "pause" );
			break;
		}

		case MS_ShowTokens:
			system( "cls" );
			GCTranslator.DumpTokens();
			system( "pause" );
			break;

		case MS_ShowUserIdentifiers:
			system( "cls" );
			GCTranslator.DumpUserIdentifiers();
			system( "pause" );
			break;

		case MS_ShowFunctions:
			system( "cls" );
			GCTranslator.DumpFunctions();
			system( "pause" );
			break;

		case MS_CallScriptFunction:
		{
			std::string     functionName;

			system( "cls" );
			printf( "Enter script function name: " );
			std::cin >> functionName;

			FFrame      frame;
			GCTranslator.ExecuteFunction( functionName, frame );
			system( "pause" );
			break;
		}
		}
	}

	return 0;
}
