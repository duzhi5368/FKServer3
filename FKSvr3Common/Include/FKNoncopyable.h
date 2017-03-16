/**
*	created:		2013-3-22   19:15
*	filename: 		FKNoncopyable
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
class zSimpNoncopyable
{
public:
	zSimpNoncopyable() {};
	~zSimpNoncopyable() {};
private:
	zSimpNoncopyable(const zSimpNoncopyable&);
	const zSimpNoncopyable & operator= (const zSimpNoncopyable &);

};
//------------------------------------------------------------------------
class zNoncopyable
{
protected:
	zNoncopyable() {};
	~zNoncopyable() {};
private:
	zNoncopyable(const zNoncopyable&);
	const zNoncopyable & operator= (const zNoncopyable &);		
};
//------------------------------------------------------------------------