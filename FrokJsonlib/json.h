/*
                                SuperEasyJSON
                    http://www.sourceforge.net/p/supereasyjson

    The MIT License (MIT)

    Copyright (c) 2013 Jeff Weinstein (jeff.weinstein at gmail)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
 */

#ifndef JSON_H
#define JSON_H

// include dependencies
//#include <assert.h>
#include <map>
#include <string>
#include <vector>

namespace json
{
    enum ValueType
    {
        NULLVal,
        StringVal,
        IntVal,
        FloatVal,
        DoubleVal,
        ObjectVal,
        ArrayVal,
        BoolVal
    };

    class Value;

    class Object
    {
        public:

            typedef std::map<std::string, Value> ValueMap;

        protected:

            ValueMap    mValues;

        public:

            Object();
            Object(const Object& obj);

            Object& operator =(const Object& obj);

            friend bool operator ==(const Object& lhs, const Object& rhs);
            inline friend bool operator !=(const Object& lhs, const Object& rhs)     {return !(lhs == rhs);}
            friend bool operator <(const Object& lhs, const Object& rhs);
            inline friend bool operator >(const Object& lhs, const Object& rhs)     {return operator<(rhs, lhs);}
            inline friend bool operator <=(const Object& lhs, const Object& rhs)    {return !operator>(lhs, rhs);}
            inline friend bool operator >=(const Object& lhs, const Object& rhs)    {return !operator<(lhs, rhs);}

            Value& operator [](const std::string& key);
            const Value& operator [](const std::string& key) const;
            Value& operator [](const char* key);
            const Value& operator [](const char* key) const;

            ValueMap::const_iterator begin() const;
            ValueMap::const_iterator end() const;
            ValueMap::iterator begin();
            ValueMap::iterator end();

            // Find will return end() if the key can't be found, just like std::map does.
            ValueMap::iterator find(const std::string& key);
            ValueMap::const_iterator find(const std::string& key) const;

            // Convenience wrapper to find to search for a key
            bool HasKey(const std::string& key) const;

            // Checks if the object contains all the keys in the array. If it does, returns -1.
            // If it doesn't, returns the index of the first key it couldn't find.
            int HasKeys(const std::vector<std::string>& keys) const;
            int HasKeys(const char* keys[], int key_count) const;

            // Removes all values and resets the state back to default
            void Clear();

            size_t size() const {return mValues.size();}

            // Erase by key
            void Erase(const std::string& key);

    };

    class Array
    {
        public:

            typedef std::vector<Value> ValueVector;

        protected:

            ValueVector                mValues;

        public:

            Array();
            Array(const Array& a);

            Array& operator =(const Array& a);

            friend bool operator ==(const Array& lhs, const Array& rhs);
            inline friend bool operator !=(const Array& lhs, const Array& rhs) {return !(lhs == rhs);}
            friend bool operator <(const Array& lhs, const Array& rhs);
            inline friend bool operator >(const Array& lhs, const Array& rhs)     {return operator<(rhs, lhs);}
            inline friend bool operator <=(const Array& lhs, const Array& rhs)    {return !operator>(lhs, rhs);}
            inline friend bool operator >=(const Array& lhs, const Array& rhs)    {return !operator<(lhs, rhs);}

            Value& operator[] (size_t i);
            const Value& operator[] (size_t i) const;

            ValueVector::const_iterator begin() const;
            ValueVector::const_iterator end() const;
            ValueVector::iterator begin();
            ValueVector::iterator end();

            // Just a convenience wrapper for doing a std::find(Array::begin(), Array::end(), Value)
            ValueVector::iterator find(const Value& v);
            ValueVector::const_iterator find(const Value& v) const;

            // Convenience wrapper to check if a value is in the array
            bool HasValue(const Value& v) const;

            // Removes all values and resets the state back to default
            void Clear();

            void push_back(const Value& v);
            void insert(size_t index, const Value& v);
            size_t size() const;
    };

    class Value
    {
        protected:

            ValueType                       mValueType;
            int                             mIntVal;
            float                           mFloatVal;
            double                          mDoubleVal;
            std::string                     mStringVal;
            Object                          mObjectVal;
            Array                           mArrayVal;
            bool                            mBoolVal;

        public:

            Value()                     : mValueType(NULLVal), mIntVal(0), mFloatVal(0), mDoubleVal(0), mBoolVal(false) {}
            Value(int v)                : mValueType(IntVal), mIntVal(v), mFloatVal((float)v), mDoubleVal((double)v) {}

            Value(float v)
            {
                mValueType = FloatVal;
                mFloatVal = v;
                mIntVal = (int)v;
                mDoubleVal = (double)v;
            }
            Value(double v)
            {
                mValueType = DoubleVal;
                mDoubleVal = v;
                mFloatVal = (float)v;
                mIntVal = (int)v;

            }
            Value(const std::string& v)    : mValueType(StringVal), mStringVal(v) {}
            Value(const char* v)        : mValueType(StringVal), mStringVal(v) {}
            Value(const Object& v)        : mValueType(ObjectVal), mObjectVal(v) {}
            Value(const Array& v)        : mValueType(ArrayVal), mArrayVal(v) {}
            Value(const bool v)            : mValueType(BoolVal), mBoolVal(v) {}
            Value(const Value& v);

            ValueType GetType() const {return mValueType;}

            Value& operator =(const Value& v);

            friend bool operator ==(const Value& lhs, const Value& rhs);
            inline friend bool operator !=(const Value& lhs, const Value& rhs)     {return !(lhs == rhs);}
            friend bool operator <(const Value& lhs, const Value& rhs);
            inline friend bool operator >(const Value& lhs, const Value& rhs)     {return operator<(rhs, lhs);}
            inline friend bool operator <=(const Value& lhs, const Value& rhs)    {return !operator>(lhs, rhs);}
            inline friend bool operator >=(const Value& lhs, const Value& rhs)    {return !operator<(lhs, rhs);}


            // For use with Array/ObjectVal types, respectively
            Value& operator [](size_t idx);
            const Value& operator [](size_t idx) const;
            Value& operator [](const std::string& key);
            const Value& operator [](const std::string& key) const;
            Value& operator [](const char* key);
            const Value& operator [](const char* key) const;

            bool         HasKey(const std::string& key) const;
            int         HasKeys(const std::vector<std::string>& keys) const;
            int         HasKeys(const char* keys[], int key_count) const;


            // non-operator versions
            int         ToInt() const        {
                if (!IsNumeric())
                    throw 0;
                return mIntVal;
            }
            float         ToFloat() const        {
                if (!IsNumeric())
                    throw 0;
                return mFloatVal;
            }
            double         ToDouble() const    {
                if (!IsNumeric())
                    throw 0;
                return mDoubleVal;
            }
            bool         ToBool() const        {
                if(!(mValueType == BoolVal))
                    throw 0;
                return mBoolVal;
            }
            std::string    ToString() const    {
                if (!(mValueType == StringVal))
                    throw 0;
                return mStringVal;
            }
            Object         ToObject() const    {
                if (mValueType != ObjectVal)
                    throw 0;

                return mObjectVal;
            }
            Array         ToArray() const        {
                if (!(mValueType == ArrayVal))
                    throw 0;
                return mArrayVal;
            }

            // Please note that as per C++ rules, implicitly casting a Value to a std::string won't work.
            // This is because it could use the int/float/double/bool operators as well. So to assign a
            // Value to a std::string you can either do:
            //         my_string = (std::string)my_value
            // Or you can now do:
            //         my_string = my_value.ToString();
            //
            operator int() const
            {
                if(!IsNumeric())
                    throw -1;
                return mIntVal;
            }
            operator float() const
            {
                if(!IsNumeric())
                    throw -1;
                return mFloatVal;
            }
            operator double() const
            {
                if(!IsNumeric())
                    throw -1;
                return mDoubleVal;
            }
            operator bool() const
            {
                if(mValueType != BoolVal)
                    throw -1;
                return mBoolVal;
            }
            operator std::string() const
            {
                if(mValueType != StringVal)
                    throw -1;
                return mStringVal;
            }
            operator Object() const
            {
                if(mValueType != ObjectVal)
                    throw -1;
                return mObjectVal;
            }
            operator Array() const
            {
                if(mValueType != ArrayVal)
                    throw -1;
                return mArrayVal;
            }

            bool IsNumeric() const             {return (mValueType == IntVal) || (mValueType == DoubleVal) || (mValueType == FloatVal);}

            // Returns 1 for anything not an Array/ObjectVal
            size_t size() const;

            // Resets the state back to default, aka NULLVal
            void Clear();

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Converts a JSON Object or Array instance into a JSON string representing it.
    std::string Serialize(const Value& obj);

    // If there is an error, Value will be NULLType
    Value         Deserialize(const std::string& str);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline bool operator ==(const Object& lhs, const Object& rhs)
    {
        return lhs.mValues == rhs.mValues;
    }

    inline bool operator <(const Object& lhs, const Object& rhs)
    {
        return lhs.mValues < rhs.mValues;
    }

    inline bool operator ==(const Array& lhs, const Array& rhs)
    {
        return lhs.mValues == rhs.mValues;
    }

    inline bool operator <(const Array& lhs, const Array& rhs)
    {
        return lhs.mValues < rhs.mValues;
    }

    /* When comparing different numeric types, this method works the same as if you compared different numeric types
     on your own. Thus it performs the same as if you, for example, did this:

         int a = 1;
         float b = 1.1f;
         bool equivalent = a == b;

        The same logic applies to the other comparison operators.
     */
    inline bool operator ==(const Value& lhs, const Value& rhs)
    {
        if ((lhs.mValueType != rhs.mValueType) && !lhs.IsNumeric() && !rhs.IsNumeric())
            return false;

        switch (lhs.mValueType)
        {
            case StringVal        :     return lhs.mStringVal == rhs.mStringVal;

            case IntVal            :     if (rhs.GetType() == FloatVal)
                                        return lhs.mIntVal == rhs.mFloatVal;
                                    else if (rhs.GetType() == DoubleVal)
                                        return lhs.mIntVal == rhs.mDoubleVal;
                                    else if (rhs.GetType() == IntVal)
                                        return lhs.mIntVal == rhs.mIntVal;
                                    else
                                        return false;

            case FloatVal        :     if (rhs.GetType() == FloatVal)
                                        return lhs.mFloatVal == rhs.mFloatVal;
                                    else if (rhs.GetType() == DoubleVal)
                                        return lhs.mFloatVal == rhs.mDoubleVal;
                                    else if (rhs.GetType() == IntVal)
                                        return lhs.mFloatVal == rhs.mIntVal;
                                    else
                                        return false;


            case DoubleVal        :     if (rhs.GetType() == FloatVal)
                                        return lhs.mDoubleVal == rhs.mFloatVal;
                                    else if (rhs.GetType() == DoubleVal)
                                        return lhs.mDoubleVal == rhs.mDoubleVal;
                                    else if (rhs.GetType() == IntVal)
                                        return lhs.mDoubleVal == rhs.mIntVal;
                                    else
                                        return false;

            case BoolVal        :     return lhs.mBoolVal == rhs.mBoolVal;

            case ObjectVal        :     return lhs.mObjectVal == rhs.mObjectVal;

            case ArrayVal        :     return lhs.mArrayVal == rhs.mArrayVal;

            default:
                return true;
        }
    }

    inline bool operator <(const Value& lhs, const Value& rhs)
    {
        if ((lhs.mValueType != rhs.mValueType) && !lhs.IsNumeric() && !rhs.IsNumeric())
            return false;

        switch (lhs.mValueType)
        {
            case StringVal        :     return lhs.mStringVal < rhs.mStringVal;

            case IntVal            :     if (rhs.GetType() == FloatVal)
                                        return lhs.mIntVal < rhs.mFloatVal;
                                    else if (rhs.GetType() == DoubleVal)
                                        return lhs.mIntVal < rhs.mDoubleVal;
                                    else if (rhs.GetType() == IntVal)
                                        return lhs.mIntVal < rhs.mIntVal;
                                    else
                                        return false;

            case FloatVal        :     if (rhs.GetType() == FloatVal)
                                        return lhs.mFloatVal < rhs.mFloatVal;
                                    else if (rhs.GetType() == DoubleVal)
                                        return lhs.mFloatVal < rhs.mDoubleVal;
                                    else if (rhs.GetType() == IntVal)
                                        return lhs.mFloatVal < rhs.mIntVal;
                                    else
                                        return false;

            case DoubleVal        :     if (rhs.GetType() == FloatVal)
                                        return lhs.mDoubleVal < rhs.mFloatVal;
                                    else if (rhs.GetType() == DoubleVal)
                                        return lhs.mDoubleVal < rhs.mDoubleVal;
                                    else if (rhs.GetType() == IntVal)
                                        return lhs.mDoubleVal < rhs.mIntVal;
                                    else
                                        return false;

            case BoolVal        :     return lhs.mBoolVal < rhs.mBoolVal;

            case ObjectVal        :     return lhs.mObjectVal < rhs.mObjectVal;

            case ArrayVal        :     return lhs.mArrayVal < rhs.mArrayVal;

            default:
                return true;
        }
    }
}

#endif //JSON_H
