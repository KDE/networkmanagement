// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
/*
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
    Copyright (c) 2003 Zack Rusin <zack@kde.org>
    Copyright (c) 2006 Michaël Larouche <michael.larouche@kdemail.net>
    Copyright (c) 2008 Allen Winter <winter@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtXml/QDomAttr>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>

#include <ostream>
#include <iostream>
#include <stdlib.h>


static inline std::ostream &operator<<(std::ostream &o, const QString &str)
{
    o << str.toLocal8Bit().constData();
    return o;
}

static void parseArgs(const QStringList &args, QString &directory, QString &file1, QString &file2)
{
    int fileCount = 0;
    directory = ".";

    for (int i = 1; i < args.count(); ++i) {
        if (args.at(i) == "-d" ||  args.at(i) == "--directory") {
            if (i + 1 > args.count()) {
                std::cerr << qPrintable(args.at(i)) << " needs an argument" << std::endl;
                exit(1);
            }
            directory = args.at(++i);
        } else if (args.at(i).startsWith("-d")) {
            directory = args.at(i).mid(2);
        } else if (args.at(i) == "--help" || args.at(i) == "-h") {
            std::cout << "Options:" << std::endl;
            std::cout << "  -L --license              Display software license" << std::endl;
            std::cout << "  -d, --directory <dir>     Directory to generate files in [.]" << std::endl;
            std::cout << "  -h, --help                Display this help" << std::endl;
            std::cout << std::endl;
            std::cout << "Arguments:" << std::endl;
            std::cout << "      file.kcfg                 Input kcfg XML file" << std::endl;
            std::cout << "      file.kcfgc                Code generation options file" << std::endl;
            exit(0);
        } else if (args.at(i) == "--license" || args.at(i) == "-L") {
            std::cout << "Copyright 2003 Cornelius Schumacher, Waldo Bastian, Zack Rusin," << std::endl;
            std::cout << "    Reinhold Kainhofer, Duncan Mac-Vicar P., Harald Fernengel" << std::endl;
            std::cout << "This program comes with ABSOLUTELY NO WARRANTY." << std::endl;
            std::cout << "You may redistribute copies of this program" << std::endl;
            std::cout << "under the terms of the GNU Library Public License." << std::endl;
            std::cout << "For more information about these matters, see the file named COPYING." << std::endl;
            exit(0);
        } else if (args.at(i).startsWith('-')) {
            std::cerr << "Unknown option: " << qPrintable(args.at(i)) << std::endl;
            exit(1);
        } else if (fileCount == 0) {
            file1 = args.at(i);
            ++fileCount;
        } else if (fileCount == 1) {
            file2 = args.at(i);
            ++fileCount;
        } else {
            std::cerr << "Too many arguments" << std::endl;
            exit(1);
        }
    }
    if (fileCount < 2) {
        std::cerr << "Too few arguments" << std::endl;
        exit(1);
    }
}

bool globalEnums;
bool useEnumTypes;
bool itemAccessors;
bool dpointer;
QStringList allNames;
QRegExp *validNameRegexp;
QString This;
QString Const;

struct SignalArguments
{
      QString type;
      QString variableName;
};

class Signal {
public:
  QString name;
  QString label;
  QList<SignalArguments> arguments;
};

class CfgEntry
{
  public:
    struct Choice
    {
      QString name;
      QString context;
      QString label;
      QString toolTip;
      QString whatsThis;
      QString dbusValue;
    };
    class Choices
    {
      public:
        Choices() {}
        Choices( const QList<Choice> &d, const QString &n, const QString &p )
             : prefix(p), choices(d), mName(n)
        {
          int i = n.indexOf("::");
          if (i >= 0)
            mExternalQual = n.left(i + 2);
        }
	QString prefix;
        QList<Choice> choices;
	const QString& name() const  { return mName; }
	const QString& externalQualifier() const  { return mExternalQual; }
	bool external() const  { return !mExternalQual.isEmpty(); }
      private:
        QString mName;
        QString mExternalQual;
    };

    CfgEntry( const QString &group, const QString &type, const QString &key,
              const QString &name, const QString &context, const QString &label,
              const QString &toolTip, const QString &whatsThis, const QString &code,
              const QString &defaultValue, const Choices &choices, const QList<Signal> signalList,
              bool hidden, bool secret, const QString & dbusKey, bool noDbus )
      : mGroup( group ), mType( type ), mKey( key ), mName( name ),
        mContext( context ), mLabel( label ), mToolTip( toolTip ), mWhatsThis( whatsThis ),
        mCode( code ), mDefaultValue( defaultValue ), mChoices( choices ),
        mSignalList(signalList), mHidden( hidden ), mSecret(secret), mDbusKey(dbusKey), mNoDbus(noDbus)
    {
        if (mDbusKey.isEmpty()) {
            mDbusKey = QString::fromLatin1("\"%1\"").arg(mName);
        }
    }

    QString dbusKey() const { return mDbusKey; }

    bool secret() const { return mSecret; }

    bool noDbus() const { return mNoDbus; }

    void setGroup( const QString &group ) { mGroup = group; }
    QString group() const { return mGroup; }

    void setType( const QString &type ) { mType = type; }
    QString type() const { return mType; }

    void setKey( const QString &key ) { mKey = key; }
    QString key() const { return mKey; }

    void setName( const QString &name ) { mName = name; }
    QString name() const { return mName; }

    void setContext( const QString &context ) { mContext = context; }
    QString context() const { return mContext; }

    void setLabel( const QString &label ) { mLabel = label; }
    QString label() const { return mLabel; }

    void setToolTip( const QString &toolTip ) { mToolTip = toolTip; }
    QString toolTip() const { return mToolTip; }

    void setWhatsThis( const QString &whatsThis ) { mWhatsThis = whatsThis; }
    QString whatsThis() const { return mWhatsThis; }

    void setDefaultValue( const QString &d ) { mDefaultValue = d; }
    QString defaultValue() const { return mDefaultValue; }

    void setCode( const QString &d ) { mCode = d; }
    QString code() const { return mCode; }

    void setMinValue( const QString &d ) { mMin = d; }
    QString minValue() const { return mMin; }

    void setMaxValue( const QString &d ) { mMax = d; }
    QString maxValue() const { return mMax; }

    void setParam( const QString &d ) { mParam = d; }
    QString param() const { return mParam; }

    void setParamName( const QString &d ) { mParamName = d; }
    QString paramName() const { return mParamName; }

    void setParamType( const QString &d ) { mParamType = d; }
    QString paramType() const { return mParamType; }

    void setChoices( const QList<Choice> &d, const QString &n, const QString &p ) { mChoices = Choices( d, n, p ); }
    Choices choices() const { return mChoices; }

    void setParamValues( const QStringList &d ) { mParamValues = d; }
    QStringList paramValues() const { return mParamValues; }

    void setParamDefaultValues( const QStringList &d ) { mParamDefaultValues = d; }
    QString paramDefaultValue(int i) const { return mParamDefaultValues[i]; }

    void setParamMax( int d ) { mParamMax = d; }
    int paramMax() const { return mParamMax; }

    void setSignalList( const QList<Signal> &value ) { mSignalList = value; }
    QList<Signal> signalList() const { return mSignalList; }

    bool hidden() const { return mHidden; }

    void dump() const
    {
      std::cerr << "<entry>" << std::endl;
      std::cerr << "  group: " << qPrintable(mGroup) << std::endl;
      std::cerr << "  type: " << qPrintable(mType) << std::endl;
      std::cerr << "  key: " << qPrintable(mKey) << std::endl;
      std::cerr << "  name: " << qPrintable(mName) << std::endl;
      std::cerr << "  context: " << qPrintable(mContext) << std::endl;
      std::cerr << "  label: " << qPrintable(mLabel) << std::endl;
// whatsthis
      std::cerr << "  code: " << qPrintable(mCode) << std::endl;
//      std::cerr << "  values: " << mValues.join(":") << std::endl;

      if (!param().isEmpty())
      {
        std::cerr << "  param name: "<< qPrintable(mParamName) << std::endl;
        std::cerr << "  param type: "<< qPrintable(mParamType) << std::endl;
        std::cerr << "  paramvalues: " << qPrintable(mParamValues.join(":")) << std::endl;
      }
      std::cerr << "  default: " << qPrintable(mDefaultValue) << std::endl;
      std::cerr << "  hidden: " << mHidden << std::endl;
      std::cerr << "  min: " << qPrintable(mMin) << std::endl;
      std::cerr << "  max: " << qPrintable(mMax) << std::endl;
      std::cerr << "</entry>" << std::endl;
    }

  private:
    QString mGroup;
    QString mType;
    QString mKey;
    QString mName;
    QString mContext;
    QString mLabel;
    QString mToolTip;
    QString mWhatsThis;
    QString mCode;
    QString mDefaultValue;
    QString mParam;
    QString mParamName;
    QString mParamType;
    Choices mChoices;
    QList<Signal> mSignalList;
    QStringList mParamValues;
    QStringList mParamDefaultValues;
    int mParamMax;
    bool mHidden;
    QString mMin;
    QString mMax;
    bool mSecret;
    QString mDbusKey;
    bool mNoDbus;
};

class Param {
public:
  QString name;
  QString type;
};

// returns the name of an member variable
// use itemPath to know the full path
// like using d-> in case of dpointer
static QString varName(const QString &n)
{
  QString result;
  if ( !dpointer ) {
    result = 'm'+n;
    result[1] = result[1].toUpper();
  }
  else {
    result = n;
    result[0] = result[0].toLower();
  }
  return result;
}

static QString varPath(const QString &n)
{
  QString result;
  if ( dpointer ) {
    result = "d->"+varName(n);
  }
  else {
    result = varName(n);
  }
  return result;
}

static QString enumName(const QString &n)
{
  QString result = "Enum" + n;
  result[4] = result[4].toUpper();
  return result;
}

static QString enumName(const QString &n, const CfgEntry::Choices &c)
{
  QString result = c.name();
  if ( result.isEmpty() )
  {
    result = "Enum" + n;
    result[4] = result[4].toUpper();
  }
  return result;
}

static QString enumType(const CfgEntry *e)
{
  QString result = e->choices().name();
  if ( result.isEmpty() )
  {
    result = "Enum" + e->name() + "::type";
    result[4] = result[4].toUpper();
  }
  return result;
}

static QString enumTypeQualifier(const QString &n, const CfgEntry::Choices &c)
{
  QString result = c.name();
  if ( result.isEmpty() )
  {
    result = "Enum" + n + "::";
    result[4] = result[4].toUpper();
  }
  else if ( c.external() )
    result = c.externalQualifier();
  else
    result.clear();
  return result;
}

static QString setFunction(const QString &n, const QString &className = QString())
{
  QString result = "set"+n;
  result[3] = result[3].toUpper();

  if ( !className.isEmpty() )
    result = className + "::" + result;
  return result;
}


static QString getFunction(const QString &n, const QString &className = QString())
{
  QString result = n;
  result[0] = result[0].toLower();

  if ( !className.isEmpty() )
    result = className + "::" + result;
  return result;
}


static void addQuotes( QString &s )
{
  if ( !s.startsWith( '"' ) ) s.prepend( '"' );
  if ( !s.endsWith( '"' ) ) s.append( '"' );
}

static QString quoteString( const QString &s )
{
  QString r = s;
  r.replace( '\\', "\\\\" );
  r.replace( '\"', "\\\"" );
  r.remove( '\r' );
  r.replace( '\n', "\\n\"\n\"" );
  return '\"' + r + '\"';
}

static QString literalString( const QString &s )
{
  bool isAscii = true;
  for(int i = s.length(); i--;)
     if (s[i].unicode() > 127) isAscii = false;

  if (isAscii)
     return "QLatin1String( " + quoteString(s) + " )";
  else
     return "QString::fromUtf8( " + quoteString(s) + " )";
}

static QString dumpNode(const QDomNode &node)
{
  QString msg;
  QTextStream s(&msg, QIODevice::WriteOnly );
  node.save(s, 0);

  msg = msg.simplified();
  if (msg.length() > 40)
    return msg.left(37)+"...";
  return msg;
}

static QString filenameOnly(const QString& path)
{
   int i = path.lastIndexOf(QRegExp("[/\\]"));
   if (i >= 0)
      return path.mid(i+1);
   return path;
}

static QString signalEnumName(const QString &signalName)
{
  QString result;
  result = "signal" + signalName;
  result[6] = result[6].toUpper();

  return result;
}

static void preProcessDefault( QString &defaultValue, const QString &name,
                               const QString &type,
                               const CfgEntry::Choices &choices,
                               QString &code )
{
    if ( type == "String" && !defaultValue.isEmpty() ) {
      defaultValue = literalString(defaultValue);

    } else if ( type == "Path" && !defaultValue.isEmpty() ) {
      defaultValue = literalString( defaultValue );

    } else if ( (type == "StringList" || type == "PathList") && !defaultValue.isEmpty() ) {
      QTextStream cpp( &code, QIODevice::WriteOnly | QIODevice::Append );
      if (!code.isEmpty())
         cpp << endl;

      cpp << "  QStringList default" << name << ";" << endl;
      const QStringList defaults = defaultValue.split( ',' );
      QStringList::ConstIterator it;
      for( it = defaults.constBegin(); it != defaults.constEnd(); ++it ) {
        cpp << "  default" << name << ".append( QString::fromUtf8( \"" << *it << "\" ) );"
            << endl;
      }
      defaultValue = "default" + name;

    } else if ( type == "Color" && !defaultValue.isEmpty() ) {
      QRegExp colorRe("\\d+,\\s*\\d+,\\s*\\d+(,\\s*\\d+)?");
      if (colorRe.exactMatch(defaultValue))
      {
        defaultValue = "QColor( " + defaultValue + " )";
      }
      else
      {
        defaultValue = "QColor( \"" + defaultValue + "\" )";
      }

    } else if ( type == "Enum" ) {
      QList<CfgEntry::Choice>::ConstIterator it;
      for( it = choices.choices.constBegin(); it != choices.choices.constEnd(); ++it ) {
        if ( (*it).name == defaultValue ) {
          if ( globalEnums && choices.name().isEmpty() )
            defaultValue.prepend( choices.prefix );
          else
            defaultValue.prepend( enumTypeQualifier(name, choices) + choices.prefix );
          break;
        }
      }

    } else if ( type == "IntList" ) {
      QTextStream cpp( &code, QIODevice::WriteOnly | QIODevice::Append );
      if (!code.isEmpty())
         cpp << endl;

      cpp << "  QList<int> default" << name << ";" << endl;
      if (!defaultValue.isEmpty())
      {
        QStringList defaults = defaultValue.split( ',' );
        QStringList::ConstIterator it;
        for( it = defaults.constBegin(); it != defaults.constEnd(); ++it ) {
          cpp << "  default" << name << ".append( " << *it << " );"
              << endl;
        }
      }
      defaultValue = "default" + name;
    }
}


CfgEntry *parseEntry( const QString &group, const QDomElement &element )
{
  bool defaultCode = false;
  QString type = element.attribute( "type" );
  QString name = element.attribute( "name" );
  QString key = element.attribute( "key" );
  QString hidden = element.attribute( "hidden" );
  QString context = element.attribute( "context" );
  bool secret = (element.attribute("secret", "false") == "true");
  QString dbusKey = element.attribute( "dbuskey" );
  if (!dbusKey.isEmpty()) {
      dbusKey = QString::fromLatin1("QLatin1String(%1)").arg(dbusKey);
  }
  bool noDbus = (element.attribute("nodbus", "false") == "true");
  QString label;
  QString toolTip;
  QString whatsThis;
  QString defaultValue;
  QString code;
  QString param;
  QString paramName;
  QString paramType;
  CfgEntry::Choices choices;
  QList<Signal> signalList;
  QStringList paramValues;
  QStringList paramDefaultValues;
  QString minValue;
  QString maxValue;
  int paramMax = 0;

  for ( QDomElement e = element.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
    QString tag = e.tagName();
    if ( tag == "label" ) {
      label = e.text();
      context = e.attribute( "context" );
    }
    else if ( tag == "tooltip" ) {
      toolTip = e.text();
      context = e.attribute( "context" );
    }
    else if ( tag == "whatsthis" ) {
      whatsThis = e.text();
      context = e.attribute( "context" );
    }
    else if ( tag == "min" ) minValue = e.text();
    else if ( tag == "max" ) maxValue = e.text();
    else if ( tag == "code" ) code = e.text();
    else if ( tag == "parameter" )
    {
      param = e.attribute( "name" );
      paramType = e.attribute( "type" );
      if ( param.isEmpty() ) {
        std::cerr << "Parameter must have a name: " << qPrintable(dumpNode(e)) << std::endl;
        return 0;
      }
      if ( paramType.isEmpty() ) {
        std::cerr << "Parameter must have a type: " << qPrintable(dumpNode(e)) << std::endl;
        return 0;
      }
      if ((paramType == "Int") || (paramType == "UInt"))
      {
         bool ok;
         paramMax = e.attribute("max").toInt(&ok);
         if (!ok)
         {
           std::cerr << "Integer parameter must have a maximum (e.g. max=\"0\"): "
                       << qPrintable(dumpNode(e)) << std::endl;
           return 0;
         }
      }
      else if (paramType == "Enum")
      {
         for ( QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement() ) {
           if (e2.tagName() == "values")
           {
             for ( QDomElement e3 = e2.firstChildElement(); !e3.isNull(); e3 = e3.nextSiblingElement() ) {
               if (e3.tagName() == "value")
               {
                  paramValues.append( e3.text() );
               }
             }
             break;
           }
         }
         if (paramValues.isEmpty())
         {
           std::cerr << "No values specified for parameter '" << qPrintable(param)
                       << "'." << std::endl;
           return 0;
         }
         paramMax = paramValues.count()-1;
      }
      else
      {
        std::cerr << "Parameter '" << qPrintable(param) << "' has type " << qPrintable(paramType)
                    << " but must be of type int, uint or Enum." << std::endl;
        return 0;
      }
    }
    else if ( tag == "default" )
    {
      if (e.attribute("param").isEmpty())
      {
        defaultValue = e.text();
        if (e.attribute( "code" ) == "true")
          defaultCode = true;
      }
    }
    else if ( tag == "choices" ) {
      QString name = e.attribute( "name" );
      QString prefix = e.attribute( "prefix" );
      QList<CfgEntry::Choice> chlist;
      for( QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement() ) {
        if ( e2.tagName() == "choice" ) {
          CfgEntry::Choice choice;
          choice.dbusValue = e2.attribute( "dbusvalue" );
          choice.name = e2.attribute( "name" );
          if ( choice.name.isEmpty() ) {
            std::cerr << "Tag <choice> requires attribute 'name'." << std::endl;
          }
          for( QDomElement e3 = e2.firstChildElement(); !e3.isNull(); e3 = e3.nextSiblingElement() ) {
            if ( e3.tagName() == "label" ) {
              choice.label = e3.text();
              choice.context = e3.attribute( "context" );
            }
            if ( e3.tagName() == "tooltip" ) {
              choice.toolTip = e3.text();
              choice.context = e3.attribute( "context" );
            }
            if ( e3.tagName() == "whatsthis" ) {
              choice.whatsThis = e3.text();
              choice.context = e3.attribute( "context" );
            }
          }
          chlist.append( choice );
        }
      }
      choices = CfgEntry::Choices( chlist, name, prefix );
    }
   else if ( tag == "emit" ) {
    QDomNode signalNode;
    Signal signal;
    signal.name = e.attribute( "signal" );
    signalList.append( signal);
   }
  }


  bool nameIsEmpty = name.isEmpty();
  if ( nameIsEmpty && key.isEmpty() ) {
    std::cerr << "Entry must have a name or a key: " << qPrintable(dumpNode(element)) << std::endl;
    return 0;
  }

  if ( key.isEmpty() ) {
    key = name;
  }

  if ( nameIsEmpty ) {
    name = key;
    name.remove( ' ' );
  } else if ( name.contains( ' ' ) ) {
    std::cout<<"Entry '"<<qPrintable(name)<<"' contains spaces! <name> elements can not contain spaces!"<<std::endl;
    name.remove( ' ' );
  }

  if (name.contains("$("))
  {
    if (param.isEmpty())
    {
      std::cerr << "Name may not be parameterized: " << qPrintable(name) << std::endl;
      return 0;
    }
  }
  else
  {
    if (!param.isEmpty())
    {
      std::cerr << "Name must contain '$(" << qPrintable(param) << ")': " << qPrintable(name) << std::endl;
      return 0;
    }
  }

  if ( label.isEmpty() ) {
    label = key;
  }

  if ( type.isEmpty() ) type = "String"; // XXX : implicit type might be bad

  if (!param.isEmpty())
  {
    // Adjust name
    paramName = name;
    name.remove("$("+param+')');
    // Lookup defaults for indexed entries
    for(int i = 0; i <= paramMax; i++)
    {
      paramDefaultValues.append(QString());
    }

    for ( QDomElement e = element.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
      QString tag = e.tagName();
      if ( tag == "default" )
      {
        QString index = e.attribute("param");
        if (index.isEmpty())
           continue;

        bool ok;
        int i = index.toInt(&ok);
        if (!ok)
        {
          i = paramValues.indexOf(index);
          if (i == -1)
          {
            std::cerr << "Index '" << qPrintable(index) << "' for default value is unknown." << std::endl;
            return 0;
          }
        }

        if ((i < 0) || (i > paramMax))
        {
          std::cerr << "Index '" << i << "' for default value is out of range [0, "<< paramMax<<"]." << std::endl;
          return 0;
        }

        QString tmpDefaultValue = e.text();

        if (e.attribute( "code" ) != "true")
           preProcessDefault(tmpDefaultValue, name, type, choices, code);

        paramDefaultValues[i] = tmpDefaultValue;
      }
    }
  }

  if (!validNameRegexp->exactMatch(name))
  {
    if (nameIsEmpty)
      std::cerr << "The key '" << qPrintable(key) << "' can not be used as name for the entry because "
                   "it is not a valid name. You need to specify a valid name for this entry." << std::endl;
    else
      std::cerr << "The name '" << qPrintable(name) << "' is not a valid name for an entry." << std::endl;
    return 0;
  }

  if (allNames.contains(name))
  {
    if (nameIsEmpty)
      std::cerr << "The key '" << qPrintable(key) << "' can not be used as name for the entry because "
                   "it does not result in a unique name. You need to specify a unique name for this entry." << std::endl;
    else
      std::cerr << "The name '" << qPrintable(name) << "' is not unique." << std::endl;
    return 0;
  }
  allNames.append(name);

  if (!defaultCode)
  {
    preProcessDefault(defaultValue, name, type, choices, code);
  }

  CfgEntry *result = new CfgEntry( group, type, key, name, context, label, toolTip, whatsThis,
                                   code, defaultValue, choices, signalList,
                                   hidden == "true", secret, dbusKey, noDbus );
  if (!param.isEmpty())
  {
    result->setParam(param);
    result->setParamName(paramName);
    result->setParamType(paramType);
    result->setParamValues(paramValues);
    result->setParamDefaultValues(paramDefaultValues);
    result->setParamMax(paramMax);
  }
  result->setMinValue(minValue);
  result->setMaxValue(maxValue);

  return result;
}

/**
  Return parameter declaration for given type.
*/
QString param( const QString &type )
{
    if ( type == "String" )           return "const QString &";
    else if ( type == "StringList" )  return "const QStringList &";
    else if ( type == "Font" )        return "const QFont &";
    else if ( type == "Rect" )        return "const QRect &";
    else if ( type == "Size" )        return "const QSize &";
    else if ( type == "Color" )       return "const QColor &";
    else if ( type == "Point" )       return "const QPoint &";
    else if ( type == "Int" )         return "int";
    else if ( type == "UInt" )        return "uint";
    else if ( type == "Bool" )        return "bool";
    else if ( type == "Double" )      return "double";
    else if ( type == "DateTime" )    return "const QDateTime &";
    else if ( type == "LongLong" )    return "qint64";
    else if ( type == "ULongLong" )   return "quint64";
    else if ( type == "IntList" )     return "const QList<int> &";
    else if ( type == "Enum" )        return "int";
    else if ( type == "Path" )        return "const QString &";
    else if ( type == "PathList" )    return "const QStringList &";
    else if ( type == "Password" )    return "const QString &";
    else if ( type == "Url" )         return "const KUrl &";
    else if ( type == "UrlList" )     return "const KUrl::List &";
    else if ( type == "ByteArray" )     return "const QByteArray &";
    else {
        std::cerr <<"kconfig_compiler does not support type \""<< type <<"\""<<std::endl;
        return "QString"; //For now, but an assert would be better
    }
}

/**
  Actual C++ storage type for given type.
*/
QString cppType( const QString &type )
{
    if ( type == "String" )           return "QString";
    else if ( type == "StringList" )  return "QStringList";
    else if ( type == "Font" )        return "QFont";
    else if ( type == "Rect" )        return "QRect";
    else if ( type == "Size" )        return "QSize";
    else if ( type == "Color" )       return "QColor";
    else if ( type == "Point" )       return "QPoint";
    else if ( type == "Int" )         return "int";
    else if ( type == "UInt" )        return "uint";
    else if ( type == "Bool" )        return "bool";
    else if ( type == "Double" )      return "double";
    else if ( type == "DateTime" )    return "QDateTime";
    else if ( type == "LongLong" )    return "qint64";
    else if ( type == "ULongLong" )   return "quint64";
    else if ( type == "IntList" )     return "QList<int>";
    else if ( type == "Enum" )        return "int";
    else if ( type == "Path" )        return "QString";
    else if ( type == "PathList" )    return "QStringList";
    else if ( type == "Password" )    return "QString";
    else if ( type == "Url" )         return "KUrl";
    else if ( type == "UrlList" )     return "KUrl::List";
    else if ( type == "ByteArray" )   return "QByteArray";
    else {
        std::cerr<<"kconfig_compiler does not support type \""<< type <<"\""<<std::endl;
        return "QString"; //For now, but an assert would be better
    }
}

QString defaultValue( const QString &type )
{
    if ( type == "String" )           return "\"\""; // Use empty string, not null string!
    else if ( type == "StringList" )  return "QStringList()";
    else if ( type == "Font" )        return "QFont()";
    else if ( type == "Rect" )        return "QRect()";
    else if ( type == "Size" )        return "QSize()";
    else if ( type == "Color" )       return "QColor(128, 128, 128)";
    else if ( type == "Point" )       return "QPoint()";
    else if ( type == "Int" )         return "0";
    else if ( type == "UInt" )        return "0";
    else if ( type == "Bool" )        return "false";
    else if ( type == "Double" )      return "0.0";
    else if ( type == "DateTime" )    return "QDateTime()";
    else if ( type == "LongLong" )    return "0";
    else if ( type == "ULongLong" )   return "0";
    else if ( type == "IntList" )     return "QList<int>()";
    else if ( type == "Enum" )        return "0";
    else if ( type == "Path" )        return "\"\""; // Use empty string, not null string!
    else if ( type == "PathList" )    return "QStringList()";
    else if ( type == "Password" )    return "\"\""; // Use empty string, not null string!
    else if ( type == "Url" )         return "KUrl()";
    else if ( type == "UrlList" )     return "KUrl::List()";
    else if ( type == "ByteArray" )     return "QByteArray()";
    else {
        std::cerr<<"Error, kconfig_compiler does not support the \""<< type <<"\" type!"<<std::endl;
        return "QString"; //For now, but an assert would be better
    }
}

QString itemType( const QString &type )
{
  QString t;

  t = type;
  t.replace( 0, 1, t.left( 1 ).toUpper() );

  return t;
}

static QString itemDeclaration(const CfgEntry *e)
{
  if (itemAccessors)
     return QString();

  QString fCap = e->name();
  fCap[0] = fCap[0].toUpper();
  return "  KConfigSkeleton::Item"+itemType( e->type() ) +
         "  *item" + fCap +
         ( (!e->param().isEmpty())?(QString("[%1]").arg(e->paramMax()+1)) : QString()) +
         ";\n";
}

// returns the name of an item variable
// use itemPath to know the full path
// like using d-> in case of dpointer
static QString itemVar(const CfgEntry *e)
{
  QString result;
  if (itemAccessors)
  {
    if ( !dpointer )
    {
      result = 'm' + e->name() + "Item";
      result[1] = result[1].toUpper();
    }
    else
    {
      result = e->name() + "Item";
      result[0] = result[0].toLower();
    }
  }
  else
  {
    result = "item" + e->name();
    result[4] = result[4].toUpper();
  }
  return result;
}

static QString itemPath(const CfgEntry *e)
{
  QString result;
  if ( dpointer ) {
    result = "d->"+itemVar(e);
  }
  else {
    result = itemVar(e);
  }
  return result;
}

QString newItem( const QString &type, const QString &name, const QString &key,
                 const QString &defaultValue, const QString &param = QString())
{
  QString t = "new KConfigSkeleton::Item" + itemType( type ) +
              "( currentGroup(), " + key + ", " + varPath( name ) + param;
  if ( type == "Enum" ) t += ", values" + name;
  if ( !defaultValue.isEmpty() ) {
    t += ", ";
    if ( type == "String" ) t += defaultValue;
    else t+= defaultValue;
  }
  t += " );";

  return t;
}

QString paramString(const QString &s, const CfgEntry *e, int i)
{
  QString result = s;
  QString needle = "$("+e->param()+')';
  if (result.contains(needle))
  {
    QString tmp;
    if (e->paramType() == "Enum")
    {
      tmp = e->paramValues()[i];
    }
    else
    {
      tmp = QString::number(i);
    }

    result.replace(needle, tmp);
  }
  return result;
}

QString paramString(const QString &group, const QList<Param> &parameters)
{
  QString paramString = group;
  QString arguments;
  int i = 1;
  for (QList<Param>::ConstIterator it = parameters.constBegin();
       it != parameters.constEnd(); ++it)
  {
     if (paramString.contains("$("+(*it).name+')'))
     {
       QString tmp;
       tmp.sprintf("%%%d", i++);
       paramString.replace("$("+(*it).name+')', tmp);
       arguments += ".arg( mParam"+(*it).name+" )";
     }
  }
  if (arguments.isEmpty())
    return "QLatin1String( \""+group+"\" )";

  return "QString( QLatin1String( \""+paramString+"\" ) )"+arguments;
}

/* int i is the value of the parameter */
QString userTextsFunctions( CfgEntry *e, QString itemVarStr=QString(), QString i=QString() )
{
  QString txt;
  if (itemVarStr.isNull()) itemVarStr=itemPath(e);
  if ( !e->label().isEmpty() ) {
    txt += "  " + itemVarStr + "->setLabel( ";
    if ( !e->context().isEmpty() )
      txt += "i18nc(" + quoteString(e->context()) + ", ";
    else
      txt += "i18n(";
    if ( !e->param().isEmpty() )
      txt += quoteString(e->label().replace("$("+e->param()+')', i));
    else
      txt+= quoteString(e->label());
    txt+= ") );\n";
  }
  if ( !e->toolTip().isEmpty() ) {
    txt += "  " + itemVarStr + "->setToolTip( ";
    if ( !e->context().isEmpty() )
      txt += "i18nc(" + quoteString(e->context()) + ", ";
    else
      txt += "i18n(";
    if ( !e->param().isEmpty() )
      txt += quoteString(e->toolTip().replace("$("+e->param()+')', i));
    else
      txt+= quoteString(e->toolTip());
    txt+=") );\n";
  }
  if ( !e->whatsThis().isEmpty() ) {
    txt += "  " + itemVarStr + "->setWhatsThis( ";
    if ( !e->context().isEmpty() )
      txt += "i18nc(" + quoteString(e->context()) + ", ";
    else
      txt += "i18n(";
    if ( !e->param().isEmpty() )
      txt += quoteString(e->whatsThis().replace("$("+e->param()+')', i));
    else
      txt+= quoteString(e->whatsThis());
    txt+=") );\n";
  }
  return txt;
}

// returns the member accesor implementation
// which should go in the h file if inline
// or the cpp file if not inline
QString memberAccessorBody( CfgEntry *e )
{
    QString result;
    QTextStream out(&result, QIODevice::WriteOnly);
    QString n = e->name();
    QString t = e->type();
    bool useEnumType = useEnumTypes && t == "Enum";

    out << "return ";
    if (useEnumType)
      out << "static_cast<" << enumType(e) << ">(";
    out << This << varPath(n);
    if (!e->param().isEmpty())
      out << "[i]";
    if (useEnumType)
      out << ")";
    out << ";" << endl;

    return result;
}

// returns the member mutator implementation
// which should go in the h file if inline
// or the cpp file if not inline
QString memberMutatorBody( CfgEntry *e )
{
  QString result;
  QTextStream out(&result, QIODevice::WriteOnly);
  QString n = e->name();
  QString t = e->type();

  if (!e->minValue().isEmpty())
  {
    out << "if (v < " << e->minValue() << ")" << endl;
    out << "{" << endl;
    out << "  kDebug() << \"" << setFunction(n);
    out << ": value \" << v << \" is less than the minimum value of ";
    out << e->minValue()<< "\" << endl;" << endl;
    out << "  v = " << e->minValue() << ";" << endl;
    out << "}" << endl;
  }

  if (!e->maxValue().isEmpty())
  {
    out << endl << "if (v > " << e->maxValue() << ")" << endl;
    out << "{" << endl;
    out << "  kDebug() << \"" << setFunction(n);
    out << ": value \" << v << \" is greater than the maximum value of ";
    out << e->maxValue()<< "\" << endl;" << endl;
    out << "  v = " << e->maxValue() << ";" << endl;
    out << "}" << endl << endl;
  }

#if 0
  out << "if (!" << This << "isImmutable( QString::fromLatin1 ( \"";
  if (!e->param().isEmpty())
  {
    out << e->paramName().replace("$("+e->param()+")", "%1") << "\" ).arg( ";
    if ( e->paramType() == "Enum" ) {
      out << "QLatin1String( ";

      if (globalEnums)
        out << enumName(e->param()) << "ToString[i]";
      else
        out << enumName(e->param()) << "::enumToString[i]";

        out << " )";
    }
    else
    {
      out << "i";
    }
    out << " )";
  }
  else
  {
    out << n << "\" )";
  }
  out << " ))" << (!e->signalList().empty() ? " {" : "") << endl;
#endif
  out << "  " << This << varPath(n);
  if (!e->param().isEmpty())
    out << "[i]";
  out << " = v;" << endl;

  if ( !e->signalList().empty() ) {
    foreach(const Signal &signal, e->signalList()) {
      out << "  " << This << varPath("settingsChanged") << " |= " << signalEnumName(signal.name) << ";" << endl;
    }
    out << "}" << endl;
  }

  return result;
}

// returns the item accesor implementation
// which should go in the h file if inline
// or the cpp file if not inline
QString itemAccessorBody( CfgEntry *e )
{
    QString result;
    QTextStream out(&result, QIODevice::WriteOnly);

    out << "return " << itemPath(e);
    if (!e->param().isEmpty()) out << "[i]";
    out << ";" << endl;

    return result;
}

//indents text adding X spaces per line
QString indent(QString text, int spaces)
{
    QString result;
    QTextStream out(&result, QIODevice::WriteOnly);
    QTextStream in(&text, QIODevice::ReadOnly);
    QString currLine;
    while ( !in.atEnd() )
    {
      currLine = in.readLine();
      if (!currLine.isEmpty())
        for (int i=0; i < spaces; i++)
          out << " ";
      out << currLine << endl;
    }
    return result;
}


int main( int argc, char **argv )
{
  QCoreApplication app(argc, argv);

  validNameRegexp = new QRegExp("[a-zA-Z_][a-zA-Z0-9_]*");

  QString directoryName, inputFilename, codegenFilename;
  parseArgs(app.arguments(), directoryName, inputFilename, codegenFilename);

  QString baseDir = directoryName;
#ifdef Q_OS_WIN
  if (!baseDir.endsWith('/') && !baseDir.endsWith('\\'))
#else
  if (!baseDir.endsWith('/'))
#endif
    baseDir.append("/");

  if (!codegenFilename.endsWith(".kcfgc"))
  {
    std::cerr << "Codegen options file must have extension .kcfgc" << std::endl;
    return 1;
  }
  QString baseName = QFileInfo(codegenFilename).fileName();
  baseName = baseName.left(baseName.length() - 6);

  QSettings codegenConfig(codegenFilename, QSettings::IniFormat);

  QString nameSpace = codegenConfig.value("NameSpace").toString();
  QString className = codegenConfig.value("ClassName").toString();
  QString inherits = codegenConfig.value("Inherits").toString();
  QString visibility = codegenConfig.value("Visibility").toString();
  QFileInfo fi(codegenFilename);
  QString settingName = fi.baseName();
  if (!visibility.isEmpty()) visibility+=' ';
  bool forceStringFilename = codegenConfig.value("ForceStringFilename", false).toBool();
  bool singleton = codegenConfig.value("Singleton", false).toBool();
  bool staticAccessors = singleton;
  //bool useDPointer = codegenConfig.readEntry("DPointer", false);
  bool customAddons = codegenConfig.value("CustomAdditions", false).toBool();
  QString memberVariables = codegenConfig.value("MemberVariables").toString();
  QStringList headerIncludes = codegenConfig.value("IncludeFiles", QStringList()).toStringList();
  QStringList sourceIncludes = codegenConfig.value("SourceIncludeFiles", QStringList()).toStringList();
  QStringList mutators = codegenConfig.value("Mutators", QStringList()).toStringList();
  bool allMutators = false;
  if ((mutators.count() == 1) && (mutators.at(0).toLower() == "true"))
     allMutators = true;
  itemAccessors = codegenConfig.value("ItemAccessors", false).toBool();
  bool setUserTexts = codegenConfig.value("SetUserTexts", false).toBool();

  globalEnums = codegenConfig.value("GlobalEnums", false).toBool();
  useEnumTypes = codegenConfig.value("UseEnumTypes", false).toBool();

  dpointer = (memberVariables == "dpointer");

  QFile input( inputFilename );

  QDomDocument doc;
  QString errorMsg;
  int errorRow;
  int errorCol;
  if ( !doc.setContent( &input, &errorMsg, &errorRow, &errorCol ) ) {
    std::cerr << "Unable to load document." << std::endl;
    std::cerr << "Parse error in " << inputFilename << ", line " << errorRow << ", col " << errorCol << ": " << errorMsg << std::endl;
    return 1;
  }

  QDomElement cfgElement = doc.documentElement();

  if ( cfgElement.isNull() ) {
    std::cerr << "No document in kcfg file" << std::endl;
    return 1;
  }

  if ( settingName.isEmpty()) {
      std::cerr << "Codegen options file does not contain SettingName field" << std::endl;
      return 1;
  }
  QString cfgFileName;
  bool cfgFileNameArg = false;
  QList<Param> parameters;
  QList<Signal> signalList;
  QStringList includes;
  bool hasSignals = false;

  QList<CfgEntry*> entries;

  for ( QDomElement e = cfgElement.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
    QString tag = e.tagName();

    if ( tag == "include" ) {
      QString includeFile = e.text();
      if (!includeFile.isEmpty())
        includes.append(includeFile);

    } else if ( tag == "kcfgfile" ) {
      cfgFileName = e.attribute( "name" );
      cfgFileNameArg = e.attribute( "arg" ).toLower() == "true";
      for( QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement() ) {
        if ( e2.tagName() == "parameter" ) {
          Param p;
          p.name = e2.attribute( "name" );
          p.type = e2.attribute( "type" );
          if (p.type.isEmpty())
             p.type = "String";
          parameters.append( p );
        }
      }

    } else if ( tag == "group" ) {
      QString group = e.attribute( "name" );
      if ( group.isEmpty() ) {
        std::cerr << "Group without name" << std::endl;
        return 1;
      }
      for( QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement() ) {
        if ( e2.tagName() != "entry" ) continue;
        CfgEntry *entry = parseEntry( group, e2 );
        if ( entry ) entries.append( entry );
        else {
          std::cerr << "Can not parse entry." << std::endl;
          return 1;
        }
      }
    }
    else if ( tag == "signal" ) {
      QString signalName = e.attribute( "name" );
      if ( signalName.isEmpty() ) {
        std::cerr << "Signal without name." << std::endl;
        return 1;
      }
      Signal theSignal;
      theSignal.name = signalName;

      for( QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement() ) {
        if ( e2.tagName() == "argument") {
          SignalArguments argument;
          argument.type = e2.attribute("type");
          if ( argument.type.isEmpty() ) {
            std::cerr << "Signal argument without type." << std::endl;
            return 1;
          }
          argument.variableName = e2.text();
          theSignal.arguments.append(argument);
        }
        else if( e2.tagName() == "label") {
          theSignal.label = e2.text();
        }
      }
      signalList.append(theSignal);
    }
  }

  if ( inherits.isEmpty() ) inherits = "Setting";

  if ( className.isEmpty() ) {
    std::cerr << "Class name missing from " << qPrintable(codegenFilename) << std::endl;
    return 1;
  }

  if ( singleton && !parameters.isEmpty() ) {
    std::cerr << "Singleton class can not have parameters" << std::endl;
    return 1;
  }

  if ( !cfgFileName.isEmpty() && cfgFileNameArg)
  {
    std::cerr << "Having both a fixed filename and a filename as argument is not possible." << std::endl;
    return 1;
  }

  if ( entries.isEmpty() ) {
    std::cerr << "No entries." << std::endl;
  }

#if 0
  CfgEntry *cfg;
  for( cfg = entries.first(); cfg; cfg = entries.next() ) {
    cfg->dump();
  }
#endif

  hasSignals = !signalList.empty();
  QString headerFileName = baseName + ".h";
  QString implementationFileName = baseName + ".cpp";
  QString mocFileName = baseName + ".moc";
  QString cppPreamble; // code to be inserted at the beginnin of the cpp file, e.g. initialization of static values

  QFile header( baseDir + headerFileName );
  if ( !header.open( QIODevice::WriteOnly ) ) {
    std::cerr << "Can not open '" << baseDir  << headerFileName << "for writing." << std::endl;
    return 1;
  }

  bool hasSecrets = false;
  QList<CfgEntry*>::ConstIterator itEntry;
  for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
    QString n = (*itEntry)->name();
    QString t = (*itEntry)->type();

    if ((*itEntry)->secret()) {
        hasSecrets = true;
        break;
    }
  }


  QTextStream h( &header );

  h << "// This file is generated by kconfig_compiler from " << QFileInfo(inputFilename).fileName() << "." << endl;
  h << "// All changes you do to this file will be lost." << endl;

  h << "#ifndef " << ( !nameSpace.isEmpty() ? nameSpace.toUpper() + '_' : "" )
    << className.toUpper() << "SETTING_H" << endl;
  h << "#define " << ( !nameSpace.isEmpty() ? nameSpace.toUpper() + '_' : "" )
    << className.toUpper() << "SETTING_H" << endl << endl;

  // Includes
  QStringList::ConstIterator it;
  for( it = headerIncludes.constBegin(); it != headerIncludes.constEnd(); ++it ) {
    if ( (*it).startsWith('"') )
      h << "#include " << *it << endl;
    else
      h << "#include <" << *it << ">" << endl;
  }

  if ( headerIncludes.count() > 0 ) h << endl;

  if ( !singleton && parameters.isEmpty() )
    h << "#include <kglobal.h>" << endl;

  h << "#include <kdebug.h>" << endl;
  h << "#include <kcoreconfigskeleton.h>" << endl;
  h << "#include \"setting.h\"" << endl;
  h << "#include \"knm_export.h\"" << endl;

  // Includes
  for( it = includes.constBegin(); it != includes.constEnd(); ++it ) {
    if ( (*it).startsWith('"') )
      h << "#include " << *it << endl;
    else
      h << "#include <" << *it << ">" << endl;
  }

  if ( !nameSpace.isEmpty() )
    h << "namespace " << nameSpace << " {" << endl << endl;

  // Private class declaration
  if ( dpointer )
    h << "class " << className << "Private;" << endl << endl;

  // Class declaration header
  h << "class " << visibility << className << "Setting : public " << inherits << endl;

  h << "{" << endl;
  // Add Q_OBJECT macro if the config need signals.
  if( hasSignals )
   h << "  Q_OBJECT" << endl;
  h << "  public:" << endl;

  // enums
  for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
    const CfgEntry::Choices &choices = (*itEntry)->choices();
    QList<CfgEntry::Choice> chlist = choices.choices;
    if ( !chlist.isEmpty() ) {
      QStringList values;
      QList<CfgEntry::Choice>::ConstIterator itChoice;
      for( itChoice = chlist.constBegin(); itChoice != chlist.constEnd(); ++itChoice ) {
        values.append( choices.prefix + (*itChoice).name );
      }
      if ( choices.name().isEmpty() ) {
        if ( globalEnums ) {
          h << "    enum { " << values.join( ", " ) << " };" << endl;
        } else {
          // Create an automatically named enum
          h << "    class " << enumName( (*itEntry)->name(), (*itEntry)->choices() ) << endl;
          h << "    {" << endl;
          h << "      public:" << endl;
          h << "      enum type { " << values.join( ", " ) << ", COUNT };" << endl;
          h << "    };" << endl;
        }
      } else if ( !choices.external() ) {
        // Create a named enum
        h << "    enum " << enumName( (*itEntry)->name(), (*itEntry)->choices() ) << " { " << values.join( ", " ) << " };" << endl;
      }
    }
    QStringList values = (*itEntry)->paramValues();
    if ( !values.isEmpty() ) {
      if ( globalEnums ) {
        // ### FIXME!!
        // make the following string table an index-based string search!
        // ###
        h << "    enum { " << values.join( ", " ) << " };" << endl;
        h << "    static const char* const " << enumName( (*itEntry)->param() ) << "ToString[];" << endl;
        cppPreamble += "const char* const " + className + "::" + enumName( (*itEntry)->param() ) +
           "ToString[] = { \"" + values.join( "\", \"" ) + "\" };\n";
      } else {
        h << "    class " << enumName( (*itEntry)->param() ) << endl;
        h << "    {" << endl;
        h << "      public:" << endl;
        h << "      enum type { " << values.join( ", " ) << ", COUNT };" << endl;
        h << "      static const char* const enumToString[];" << endl;
        h << "    };" << endl;
        cppPreamble += "const char* const " + className + "::" + enumName( (*itEntry)->param() ) +
           "::enumToString[] = { \"" + values.join( "\", \"" ) + "\" };\n";
      }
    }
  }
  if ( hasSignals ) {
   h << "\n    enum {" << endl;
   unsigned val = 1;
   QList<Signal>::ConstIterator it, itEnd = signalList.constEnd();
   for ( it = signalList.constBegin(); it != itEnd; val <<= 1) {
     if ( !val ) {
       std::cerr << "Too many signals to create unique bit masks" << std::endl;
       exit(1);
     }
     Signal signal = *it;
     h << "      " << signalEnumName(signal.name) << " = 0x" << hex << val;
     if ( ++it != itEnd )
      h << ",";
     h << endl;
   }
   h << " };" << dec << endl;
  }
  h << endl;
  // Constructor or singleton accessor
  if ( !singleton ) {
    h << "    " << className << "Setting(";
    if (cfgFileNameArg)
    {
        if(forceStringFilename)
            h << " const QString &cfgfilename"
                << (parameters.isEmpty() ? " = QString()" : ", ");
        else
            h << " KSharedConfig::Ptr config"
                << (parameters.isEmpty() ? " = KGlobal::config()" : ", ");
    }
    for (QList<Param>::ConstIterator it = parameters.constBegin();
         it != parameters.constEnd(); ++it)
    {
       if (it != parameters.constBegin())
         h << ",";
       h << " " << param((*it).type) << " " << (*it).name;
    }
    h << " );" << endl;
  } else {
    h << "    static " << className << " *self();" << endl;
    if (cfgFileNameArg)
    {
      h << "    static void instance(const QString& cfgfilename);" << endl;
    }
  }

  // Destructor
  h << "    ~" << className << "Setting();" << endl << endl;

  // name accessor
  h << "    QString name() const;" << endl << endl;
  h << "    bool hasSecrets() const;" << endl << endl;

  // global variables
  if (staticAccessors)
    This = "self()->";
  else
    Const = " const";

  for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
    QString n = (*itEntry)->name();
    QString t = (*itEntry)->type();

    // Manipulator
    if (allMutators || mutators.contains(n))
    {
      h << "    /**" << endl;
      h << "      Set " << (*itEntry)->label() << endl;
      h << "    */" << endl;
      if (staticAccessors)
        h << "    static" << endl;
      h << "    void " << setFunction(n) << "( ";
      if (!(*itEntry)->param().isEmpty())
        h << cppType((*itEntry)->paramType()) << " i, ";
      if (useEnumTypes && t == "Enum")
        h << enumType(*itEntry);
      else
        h << param( t );
      h << " v )";
      // function body inline only if not using dpointer
      // for BC mode
      if ( !dpointer )
      {
        h << endl << "    {" << endl;
        h << indent(memberMutatorBody(*itEntry), 6 );
        h << "    }" << endl;
      }
      else
      {
        h << ";" << endl;
      }
    }
    h << endl;
    // Accessor
    h << "    /**" << endl;
    h << "      Get " << (*itEntry)->label() << endl;
    h << "    */" << endl;
    if (staticAccessors)
      h << "    static" << endl;
    h << "    ";
    if (useEnumTypes && t == "Enum")
      h << enumType(*itEntry);
    else
      h << cppType(t);
    h << " " << getFunction(n) << "(";
    if (!(*itEntry)->param().isEmpty())
      h << " " << cppType((*itEntry)->paramType()) <<" i ";
    h << ")" << Const;
    // function body inline only if not using dpointer
    // for BC mode
    if ( !dpointer )
    {
       h << endl << "    {" << endl;
      h << indent(memberAccessorBody((*itEntry)), 6 );
       h << "    }" << endl;
    }
    else
    {
      h << ";" << endl;
    }

    // Item accessor
    if ( itemAccessors ) {
      h << endl;
      h << "    /**" << endl;
      h << "      Get Item object corresponding to " << n << "()"
        << endl;
      h << "    */" << endl;
      h << "    Item" << itemType( (*itEntry)->type() ) << " *"
        << getFunction( n ) << "Item(";
      if (!(*itEntry)->param().isEmpty()) {
        h << " " << cppType((*itEntry)->paramType()) << " i ";
      }
      h << ")";
      if (! dpointer )
      {
        h << endl << "    {" << endl;
        h << indent( itemAccessorBody((*itEntry)), 6);
        h << "    }" << endl;
      }
      else
      {
        h << ";" << endl;
      }
    }

    h << endl;
  }


  // Signal definition.
  if( hasSignals ) {
    h << endl;
    h << "  Q_SIGNALS:";
    foreach(const Signal &signal, signalList) {
      h << endl;
      if ( !signal.label.isEmpty() ) {
        h << "    /**" << endl;
        h << "      " << signal.label << endl;
        h << "    */" << endl;
      }
      h << "    void " << signal.name << "(";
      QList<SignalArguments>::ConstIterator it, itEnd = signal.arguments.constEnd();
      for ( it = signal.arguments.constBegin(); it != itEnd; ) {
        SignalArguments argument = *it;
        QString type = param(argument.type);
        if ( useEnumTypes && argument.type == "Enum" ) {
          for ( int i = 0, end = entries.count(); i < end; ++i ) {
            if ( entries[i]->name() == argument.variableName ) {
              type = enumType(entries[i]);
              break;
            }
          }
        }
        h << type << " " << argument.variableName;
        if ( ++it != itEnd ) {
         h << ", ";
        }
      }
      h << ");" << endl;
    }
    h << endl;
  }

  h << "  protected:" << endl;

  // Private constructor for singleton
  if ( singleton ) {
    h << "    " << className << "(";
    if ( cfgFileNameArg )
      h << "const QString& arg";
    h << ");" << endl;
    h << "    friend class " << className << "Helper;" << endl << endl;
  }

  if ( hasSignals ) {
    h << "    virtual void usrWriteConfig();" << endl;
  }

  // Member variables
  if ( !memberVariables.isEmpty() && memberVariables != "private" && memberVariables != "dpointer") {
    h << "  " << memberVariables << ":" << endl;
  }

  // Class Parameters
  for (QList<Param>::ConstIterator it = parameters.constBegin();
       it != parameters.constEnd(); ++it)
  {
     h << "    " << cppType((*it).type) << " mParam" << (*it).name << ";" << endl;
  }

  if ( memberVariables != "dpointer" )
  {
    QString group;
    for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
      if ( (*itEntry)->group() != group ) {
        group = (*itEntry)->group();
        h << endl;
        h << "    // " << group << endl;
      }
      h << "    " << cppType( (*itEntry)->type() ) << " " << varName( (*itEntry)->name() );
      if ( !(*itEntry)->param().isEmpty() )
      {
        h << QString("[%1]").arg( (*itEntry)->paramMax()+1 );
      }
      h << ";" << endl;
    }

    h << endl << "  private:" << endl;
    if ( itemAccessors ) {
       for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
        h << "    Item" << itemType( (*itEntry)->type() ) << " *" << itemVar( *itEntry );
        if ( !(*itEntry)->param().isEmpty() ) h << QString("[%1]").arg( (*itEntry)->paramMax()+1 );
        h << ";" << endl;
      }
    }
    if ( hasSignals )
     h << "    uint " << varName("settingsChanged") << ";" << endl;

  }
  else
  {
    // use a private class for both member variables and items
    h << "  private:" << endl;
    h << "    " + className + "Private *d;" << endl;
  }

  if (customAddons)
  {
     h << "    // Include custom additions" << endl;
     h << "    #include \"" << filenameOnly(baseName) << "_addons.h\"" <<endl;
  }

  h << "};" << endl << endl;

  if ( !nameSpace.isEmpty() ) h << "}" << endl << endl;

  h << "#endif" << endl << endl;


  header.close();

  QFile implementation( baseDir + implementationFileName );
  if ( !implementation.open( QIODevice::WriteOnly ) ) {
    std::cerr << "Can not open '" << qPrintable(implementationFileName) << "for writing."
              << std::endl;
    return 1;
  }

  QTextStream cpp( &implementation );


  cpp << "// This file is generated by kconfig_compiler from " << QFileInfo(inputFilename).fileName() << "." << endl;
  cpp << "// All changes you do to this file will be lost." << endl << endl;

  cpp << "#include \"" << headerFileName << "\"" << endl << endl;

  for( it = sourceIncludes.constBegin(); it != sourceIncludes.constEnd(); ++it ) {
    if ( (*it).startsWith('"') )
      cpp << "#include " << *it << endl;
    else
      cpp << "#include <" << *it << ">" << endl;
  }

  if ( sourceIncludes.count() > 0 ) cpp << endl;

  if ( setUserTexts ) cpp << "#include <klocale.h>" << endl << endl;

  // Header required by singleton implementation
  if ( singleton )
    cpp << "#include <kglobal.h>" << endl << "#include <QtCore/QFile>" << endl << endl;
  if ( singleton && cfgFileNameArg )
    cpp << "#include <kdebug.h>" << endl << endl;

  if ( !nameSpace.isEmpty() )
    cpp << "using namespace " << nameSpace << ";" << endl << endl;

  QString group;

  // private class implementation
  if ( dpointer )
  {
    if ( !nameSpace.isEmpty() )
      cpp << "namespace " << nameSpace << " {" << endl;
    cpp << "class " << className << "Private" << endl;
    cpp << "{" << endl;
    cpp << "  public:" << endl;
    for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
      if ( (*itEntry)->group() != group ) {
        group = (*itEntry)->group();
        cpp << endl;
        cpp << "    // " << group << endl;
      }
      cpp << "    " << cppType( (*itEntry)->type() ) << " " << varName( (*itEntry)->name() );
      if ( !(*itEntry)->param().isEmpty() )
      {
        cpp << QString("[%1]").arg( (*itEntry)->paramMax()+1 );
      }
      cpp << ";" << endl;
    }
    cpp << endl << "    // items" << endl;
    for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
      cpp << "    KConfigSkeleton::Item" << itemType( (*itEntry)->type() ) << " *" << itemVar( *itEntry );
      if ( !(*itEntry)->param().isEmpty() ) cpp << QString("[%1]").arg( (*itEntry)->paramMax()+1 );
        cpp << ";" << endl;
    }
    if ( hasSignals ) {
      cpp << "    uint " << varName("settingsChanged") << ";" << endl;
    }

    cpp << "};" << endl << endl;
    if ( !nameSpace.isEmpty() )
      cpp << "}" << endl << endl;
  }

  // Singleton implementation
  if ( singleton ) {
    if( !nameSpace.isEmpty() )
      cpp << "namespace " << nameSpace << " {" << endl;
    cpp << "class " << className << "Helper" << endl;
    cpp << '{' << endl;
    cpp << "  public:" << endl;
    cpp << "    " << className << "Helper() : q(0) {}" << endl;
    cpp << "    ~" << className << "Helper() { delete q; }" << endl;
    cpp << "    " << className << " *q;" << endl;
    cpp << "};" << endl;
    if( !nameSpace.isEmpty() )
      cpp << "}" << endl;
    cpp << "K_GLOBAL_STATIC(" << className << "Helper, s_global" << className << ")" << endl;

    cpp << className << " *" << className << "::self()" << endl;
    cpp << "{" << endl;
    if ( cfgFileNameArg ) {
      cpp << "  if (!s_global" << className << "->q)" << endl;
      cpp << "     kFatal() << \"you need to call " << className << "::instance before using\";" << endl;
    } else {
      cpp << "  if (!s_global" << className << "->q) {" << endl;
      cpp << "    new " << className << ';' << endl;
      cpp << "    s_global" << className << "->q->readConfig();" << endl;
      cpp << "  }" << endl << endl;
    }
    cpp << "  return s_global" << className << "->q;" << endl;
    cpp << "}" << endl << endl;

    if ( cfgFileNameArg ) {
      cpp << "void " << className << "::instance(const QString& cfgfilename)" << endl;
      cpp << "{" << endl;
      cpp << "  if (s_global" << className << "->q) {" << endl;
      cpp << "     kDebug() << \"" << className << "::instance called after the first use - ignoring\";" << endl;
      cpp << "     return;" << endl;
      cpp << "  }" << endl;
      cpp << "  new " << className << "(cfgfilename);" << endl;
      cpp << "  s_global" << className << "->q->readConfig();" << endl;
      cpp << "}" << endl << endl;
    }
  }

  if ( !cppPreamble.isEmpty() )
    cpp << cppPreamble << endl;

  // Constructor
  cpp << className << "Setting::" << className << "Setting() : Setting(Setting::" <<className << ")" << endl;
  if ( cfgFileNameArg ) {
    if ( !singleton && ! forceStringFilename)
      cpp << " KSharedConfig::Ptr config";
    else
      cpp << " const QString& config";
    cpp << (parameters.isEmpty() ? " " : ", ");
  }

  for (QList<Param>::ConstIterator it = parameters.constBegin();
       it != parameters.constEnd(); ++it)
  {
     if (it != parameters.constBegin())
       cpp << ",";
     cpp << " " << param((*it).type) << " " << (*it).name;
  }
  //cpp << " )" << endl;

  //cpp << "  : " << inherits << "(";
  if ( !cfgFileName.isEmpty() ) cpp << " QLatin1String( \"" << cfgFileName << "\" ";
  if ( cfgFileNameArg ) cpp << " config ";
  if ( !cfgFileName.isEmpty() ) cpp << ") ";
  //cpp << ")" << endl;

  // Store parameters
  for (QList<Param>::ConstIterator it = parameters.constBegin();
       it != parameters.constEnd(); ++it)
  {
     cpp << "  , mParam" << (*it).name << "(" << (*it).name << ")" << endl;
  }

  cpp << "{" << endl;

#if 0
  if (dpointer)
    cpp << "  d = new " + className + "Private;" << endl;
  // Needed in case the singleton class is used as baseclass for
  // another singleton.
  if (singleton) {
    cpp << "  Q_ASSERT(!s_global" << className << "->q);" << endl;
    cpp << "  s_global" << className << "->q = this;" << endl;
  }

  group.clear();

  for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
    if ( (*itEntry)->group() != group ) {
      if ( !group.isEmpty() ) cpp << endl;
      group = (*itEntry)->group();
      cpp << "  setCurrentGroup( " << paramString(group, parameters) << " );" << endl << endl;
    }

    QString key = paramString( (*itEntry)->key(), parameters );
    if ( !(*itEntry)->code().isEmpty() ) {
      cpp << (*itEntry)->code() << endl;
    }
    if ( (*itEntry)->type() == "Enum" ) {
      cpp << "  QList<KConfigSkeleton::ItemEnum::Choice2> values"
          << (*itEntry)->name() << ";" << endl;
      QList<CfgEntry::Choice> choices = (*itEntry)->choices().choices;
      QList<CfgEntry::Choice>::ConstIterator it;
      for( it = choices.constBegin(); it != choices.constEnd(); ++it ) {
        cpp << "  {" << endl;
        cpp << "    KConfigSkeleton::ItemEnum::Choice2 choice;" << endl;
        cpp << "    choice.name = QLatin1String(\"" << (*it).name << "\");" << endl;
        if ( setUserTexts ) {
          if ( !(*it).label.isEmpty() ) {
            cpp << "    choice.label = ";
            if ( !(*it).context.isEmpty() )
              cpp << "i18nc(" + quoteString((*it).context) + ", ";
            else
              cpp << "i18n(";
            cpp << quoteString((*it).label) << ");" << endl;
          }
          if ( !(*it).toolTip.isEmpty() ) {
            cpp << "    choice.toolTip = ";
            if ( !(*it).context.isEmpty() )
              cpp << "i18nc(" + quoteString((*it).context) + ", ";
            else
              cpp << "i18n(";
            cpp << quoteString((*it).toolTip) << ");" << endl;
          }
          if ( !(*it).whatsThis.isEmpty() ) {
            cpp << "    choice.whatsThis = ";
            if ( !(*it).context.isEmpty() )
              cpp << "i18nc(" + quoteString((*it).context) + ", ";
            else
              cpp << "i18n(";
            cpp << quoteString((*it).whatsThis) << ");" << endl;
          }
        }
        cpp << "    values" << (*itEntry)->name() << ".append( choice );" << endl;
        cpp << "  }" << endl;
      }
    }

    if (!dpointer)
      cpp << itemDeclaration( *itEntry );

    if ( (*itEntry)->param().isEmpty() )
    {
      // Normal case
      cpp << "  " << itemPath( *itEntry ) << " = "
          << newItem( (*itEntry)->type(), (*itEntry)->name(), key, (*itEntry)->defaultValue() ) << endl;

      if ( !(*itEntry)->minValue().isEmpty() )
        cpp << "  " << itemPath( *itEntry ) << "->setMinValue(" << (*itEntry)->minValue() << ");" << endl;
      if ( !(*itEntry)->maxValue().isEmpty() )
        cpp << "  " << itemPath( *itEntry ) << "->setMaxValue(" << (*itEntry)->maxValue() << ");" << endl;

      if ( setUserTexts )
        cpp << userTextsFunctions( (*itEntry) );

      cpp << "  addItem( " << itemPath( *itEntry );
      QString quotedName = (*itEntry)->name();
      addQuotes( quotedName );
      if ( quotedName != key ) cpp << ", QLatin1String( \"" << (*itEntry)->name() << "\" )";
      cpp << " );" << endl;
    }
    else
    {
      // Indexed
      for(int i = 0; i <= (*itEntry)->paramMax(); i++)
      {
        QString defaultStr;
        QString itemVarStr(itemPath( *itEntry )+QString("[%1]").arg(i));

        if ( !(*itEntry)->paramDefaultValue(i).isEmpty() )
          defaultStr = (*itEntry)->paramDefaultValue(i);
        else if ( !(*itEntry)->defaultValue().isEmpty() )
          defaultStr = paramString( (*itEntry)->defaultValue(), (*itEntry), i );
        else
          defaultStr = defaultValue( (*itEntry)->type() );

        cpp << "  " << itemVarStr << " = "
            << newItem( (*itEntry)->type(), (*itEntry)->name(), paramString(key, *itEntry, i), defaultStr, QString("[%1]").arg(i) )
            << endl;

        if ( setUserTexts )
          cpp << userTextsFunctions( *itEntry, itemVarStr, (*itEntry)->paramName() );

        // Make mutators for enum parameters work by adding them with $(..) replaced by the
        // param name. The check for isImmutable in the set* functions doesn't have the param
        // name available, just the corresponding enum value (int), so we need to store the
        // param names in a separate static list!.
        cpp << "  addItem( " << itemVarStr << ", QLatin1String( \"";
        if ( (*itEntry)->paramType()=="Enum" )
          cpp << (*itEntry)->paramName().replace( "$("+(*itEntry)->param()+')', "%1").arg((*itEntry)->paramValues()[i] );
        else
          cpp << (*itEntry)->paramName().replace( "$("+(*itEntry)->param()+')', "%1").arg(i);
        cpp << "\" ) );" << endl;
      }
    }
  }
#endif
  cpp << "}" << endl << endl;

  if (dpointer)
  {
    // setters and getters go in Cpp if in dpointer mode
    for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
      QString n = (*itEntry)->name();
      QString t = (*itEntry)->type();

      // Manipulator
      if (allMutators || mutators.contains(n))
      {
        cpp << "void " << setFunction(n, className) << "( ";
        if ( !(*itEntry)->param().isEmpty() )
          cpp << cppType( (*itEntry)->paramType() ) << " i, ";
        if (useEnumTypes && t == "Enum")
          cpp << enumType(*itEntry);
        else
          cpp << param( t );
        cpp << " v )";
        // function body inline only if not using dpointer
        // for BC mode
        cpp << "{" << endl;
        cpp << indent(memberMutatorBody( *itEntry ), 6);
        cpp << "}" << endl << endl;
      }

      // Accessor
      if (useEnumTypes && t == "Enum")
        cpp << enumType(*itEntry);
      else
        cpp << cppType(t);
      cpp << " " << getFunction(n, className) << "(";
      if ( !(*itEntry)->param().isEmpty() )
        cpp << " " << cppType( (*itEntry)->paramType() ) <<" i ";
      cpp << ")" << Const << endl;
      // function body inline only if not using dpointer
      // for BC mode
      cpp << "{" << endl;
      cpp << indent(memberAccessorBody( *itEntry ), 2);
      cpp << "}" << endl << endl;

      // Item accessor
      if ( itemAccessors )
      {
        cpp << endl;
        cpp << "KConfigSkeleton::Item" << itemType( (*itEntry)->type() ) << " *"
          << getFunction( n, className ) << "Item(";
        if ( !(*itEntry)->param().isEmpty() ) {
          cpp << " " << cppType( (*itEntry)->paramType() ) << " i ";
        }
        cpp << ")" << endl;
        cpp << "{" << endl;
        cpp << indent(itemAccessorBody( *itEntry ), 2);
        cpp << "}" << endl;
      }

      cpp << endl;
    }
  }

  // Destructor
  cpp << className << "Setting::~" << className << "Setting()" << endl;
  cpp << "{" << endl;
  if ( singleton ) {
    if ( dpointer )
      cpp << "  delete d;" << endl;
    cpp << "  if (!s_global" << className << ".isDestroyed()) {" << endl;
    cpp << "    s_global" << className << "->q = 0;" << endl;
    cpp << "  }" << endl;
  }
  cpp << "}" << endl << endl;

  if ( hasSignals ) {
    cpp << "void " << className << "::" << "usrWriteConfig()" << endl;
    cpp << "{" << endl;
    cpp << "  " << inherits << "::usrWriteConfig();" << endl << endl;
    foreach(const Signal &signal, signalList) {
      cpp << "  if ( " << varPath("settingsChanged") << " & " << signalEnumName(signal.name) << " ) " << endl;
      cpp << "    emit " << signal.name << "(";
      QList<SignalArguments>::ConstIterator it, itEnd = signal.arguments.constEnd();
      for ( it = signal.arguments.constBegin(); it != itEnd; ) {
        SignalArguments argument = *it;
        bool cast = false;
        if ( useEnumTypes && argument.type == "Enum" ) {
          for ( int i = 0, end = entries.count(); i < end; ++i ) {
            if ( entries[i]->name() == argument.variableName ) {
              cpp << "static_cast<" << enumType(entries[i]) << ">(";
              cast = true;
              break;
            }
          }
        }
        cpp << varPath(argument.variableName);
        if ( cast )
          cpp << ")";
        if ( ++it != itEnd )
          cpp << ", ";
      }
      cpp << ");" << endl << endl;
    }
    cpp << "  " << varPath("settingsChanged") << " = 0;" << endl;
    cpp << "}" << endl;
  }

  cpp << "QString " <<  className << "Setting::name() const" << endl;
  cpp << "{" << endl;
  cpp << "  return QLatin1String(\"" << settingName << "\");" << endl;
  cpp << "}" << endl;

  cpp << "bool " <<  className << "Setting::hasSecrets() const" << endl;
  cpp << "{" << endl;
  cpp << "  return " <<(hasSecrets ? "true;" : "false;" ) << endl;
  cpp << "}" << endl;

  // Add includemoc if they are signals defined.
  if( hasSignals ) {
    cpp << endl;
    cpp << "#include \"" << mocFileName << "\"" << endl;
    cpp << endl;
  }

  implementation.close();

  // persistence helper
  QString persistenceHeaderFilename = baseName + "persistence.h";
  QString persistenceFileName = baseName + "persistence.cpp";
  QFile persistenceHeader( baseDir + persistenceHeaderFilename );
  if ( !persistenceHeader.open( QIODevice::WriteOnly ) ) {
    std::cerr << "Can not open '" << qPrintable(persistenceHeaderFilename) << "for writing."
              << std::endl;
    return 1;
  }

  QTextStream pH( &persistenceHeader );

  inherits = "SettingPersistence";

  pH << "// This file is generated by kconfig_compiler from " << QFileInfo(inputFilename).fileName() << "." << endl;
  pH << "// All changes you do to this file will be lost." << endl;

  pH << "#ifndef " << ( !nameSpace.isEmpty() ? nameSpace.toUpper() + '_' : "" )
    << className.toUpper() << "PERSISTENCE_H" << endl;
  pH << "#define " << ( !nameSpace.isEmpty() ? nameSpace.toUpper() + '_' : "" )
    << className.toUpper() << "PERSISTENCE_H" << endl << endl;

  pH << "#include <kdebug.h>" << endl;
  pH << "#include <kcoreconfigskeleton.h>" << endl;
  pH << "#include \"settingpersistence.h\"" << endl;
  pH << "#include \"knm_export.h\"" << endl;

  if ( !nameSpace.isEmpty() )
    pH << "namespace " << nameSpace << " {" << endl << endl;

  pH << "class " << className << "Setting;" << endl << endl;
  // Class declaration header
  pH << "class " << visibility << className << "Persistence : public " << inherits << endl;
  pH << "{" << endl;
  pH << "  public:" << endl;
  pH << "    " << className << "Persistence( " << className << "Setting * setting, KSharedConfig::Ptr config, ConnectionPersistence::SecretStorageMode mode = ConnectionPersistence::Secure);" << endl;
  pH << "    ~" << className << "Persistence();" << endl;
  pH << "    void load();" << endl;
  pH << "    void save();" << endl;
  pH << "    QMap<QString,QString> secrets() const;" << endl;
  pH << "    void restoreSecrets(QMap<QString,QString>) const;" << endl;
  pH << "};" << endl;

  if ( !nameSpace.isEmpty() ) pH << "}" << endl << endl;

  pH << "#endif" << endl << endl;

  persistenceHeader.close();

  QFile persImplementation( baseDir + persistenceFileName );
  if ( !persImplementation.open( QIODevice::WriteOnly ) ) {
    std::cerr << "Can not open '" << qPrintable(persistenceFileName) << "for writing."
              << std::endl;
    return 1;
  }

  QTextStream pC( &persImplementation );


  pC << "// This file is generated by kconfig_compiler from " << QFileInfo(inputFilename).fileName() << "." << endl;
  pC << "// All changes you do to this file will be lost." << endl << endl;

  pC << "#include \"" << persistenceHeaderFilename << "\"" << endl << endl;
  pC << "#include \"" << headerFileName << "\"" << endl << endl;

  if ( !nameSpace.isEmpty() )
    pC << "using namespace " << nameSpace << ";" << endl << endl;

  // Constructor
  pC << className << "Persistence::" << className << "Persistence(" << className << "Setting * setting, KSharedConfig::Ptr config, ConnectionPersistence::SecretStorageMode mode) : SettingPersistence(setting, config, mode)" << endl;
  pC << "{" << endl;
  pC << "}" << endl << endl;

  // Destructor
  pC << className << "Persistence::~" << className << "Persistence()" << endl;
  pC << "{" << endl;
  pC << "}" << endl << endl;

  // load method
  pC << "void " << className << "Persistence::load()" << endl;
  pC << "{" << endl;
  pC << "  " << className << "Setting * setting = static_cast<" << className << "Setting *>(m_setting);" << endl;
  for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
    QString n = (*itEntry)->name();
    QString t = (*itEntry)->type();

    QString defaultStr;
    if ( !(*itEntry)->defaultValue().isEmpty() )
        defaultStr = (*itEntry)->defaultValue();
    else
        defaultStr = defaultValue( (*itEntry)->type() );

    if ((*itEntry)->secret()) {
        pC << "  // SECRET" << endl;
        pC << "  if (m_storageMode != ConnectionPersistence::Secure) {" << endl << "  ";
    }
    if (t == "Enum") {
        const CfgEntry::Choices &choices = (*itEntry)->choices();
        QList<CfgEntry::Choice> chlist = choices.choices;
        if ( chlist.isEmpty() ) {
            // as non-enum
            pC << "  setting->" << setFunction(n) << "(m_config->readEntry(\"" << (*itEntry)->key() << "\", " << defaultStr << "));" << endl;
        } else { // switch the value and write the choice name as string instead of the enum's int value
            pC << "  {" << endl; // keep contents in this scope
            pC << "    QString contents = m_config->readEntry(\"" << (*itEntry)->key() << "\", " << defaultStr << ");" << endl;
            for (int i = 0; i < chlist.count(); i++) {
                CfgEntry::Choice ch = chlist[i];
                pC << "    if (contents == \"" << ch.name << "\")" << endl;
                pC << "      setting->" << setFunction(n) << "(" << className << "Setting::" << enumName(n, choices) << "::" << choices.prefix << ch.name << ");" << endl;
                if (i != (chlist.count() - 1)) {
                    pC << "    else ";
                } else {
                    pC << endl;
                }
            }
            pC << "  }" << endl; // keep contents in this scope
        }
    } else {
        pC << "  setting->" << setFunction(n) << "(m_config->readEntry(\"" << (*itEntry)->key() << "\", " << defaultStr << "));" << endl;
    }
    if ((*itEntry)->secret()) {
        pC << "  }" << endl;
    }
  }
  pC << "}" << endl << endl;

  // save method
  pC << "void " << className << "Persistence::save()" << endl;
  pC << "{" << endl;
  pC << "  " << className << "Setting * setting = static_cast<" << className << "Setting *>(m_setting);" << endl;
  for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
    QString n = (*itEntry)->name();
    QString t = (*itEntry)->type();

    QString defaultStr;
    if ( !(*itEntry)->defaultValue().isEmpty() )
        defaultStr = (*itEntry)->defaultValue();
    else
        defaultStr = defaultValue( (*itEntry)->type() );

    if ((*itEntry)->secret()) {
        pC << "  // SECRET" << endl;
        pC << "  if (m_storageMode != ConnectionPersistence::Secure) {" << endl << "  ";
    }
    if (t == "Enum") {
        const CfgEntry::Choices &choices = (*itEntry)->choices();
        QList<CfgEntry::Choice> chlist = choices.choices;
        if ( chlist.isEmpty() ) {
            // as non-enum
            pC << "  m_config->writeEntry(\"" << (*itEntry)->key() << "\", setting->" << getFunction(n) << "());" << endl;
        } else { // switch the value and write the choice name as string instead of the enum's int value
            pC << "  switch (setting->" << getFunction(n) << "()) {" << endl;
            foreach (CfgEntry::Choice ch, chlist) {
                pC << "    case " << className << "Setting::" << enumName(n, choices) << "::" << choices.prefix << ch.name << ":" << endl;
                pC << "      m_config->writeEntry(\"" << (*itEntry)->key() << "\", \"" << ch.name << "\");" << endl;
                pC << "      break;" << endl;
            }
            pC << "  }" << endl;
        }
    } else {
        pC << "  m_config->writeEntry(\"" << (*itEntry)->key() << "\", setting->" << getFunction(n) << "());" << endl;
    }
    if ((*itEntry)->secret()) {
        pC << "  }" << endl;
    }
  }
  pC << "}" << endl << endl;

  // secrets() method
  pC << "QMap<QString,QString> " << className << "Persistence::secrets() const" << endl;
  pC << "{" << endl;
  if (hasSecrets) {
      pC << "  " << className << "Setting * setting = static_cast<" << className << "Setting *>(m_setting);" << endl;
  }
  pC << "  QMap<QString,QString> map;" << endl;

  for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
    QString n = (*itEntry)->name();
    QString t = (*itEntry)->type();

    if ((*itEntry)->secret()) {
        pC << "  map.insert(QLatin1String(\"" << (*itEntry)->key() << "\"), setting->" << getFunction(n) << "());" << endl;
    }
  }
  pC << "  return map;" << endl;

  pC << "}" << endl << endl;

  // restoreSecrets()
  pC << "void " << className << "Persistence::restoreSecrets(QMap<QString,QString> secrets) const" << endl;
  pC << "{" << endl;
  pC << "  if (m_storageMode == ConnectionPersistence::Secure) {" << endl;
  if (hasSecrets) {
      pC << "  " << className << "Setting * setting = static_cast<" << className << "Setting *>(m_setting);" << endl;
  } else {
      pC << "  Q_UNUSED(secrets);" << endl;
  }
  for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
    QString n = (*itEntry)->name();
    QString t = (*itEntry)->type();

    if ((*itEntry)->secret()) {
        pC << "    setting->" << setFunction(n) << "(secrets.value(\"" << n << "\"));" << endl;
    }
  }
  if (hasSecrets) {
    pC << "    setting->setSecretsAvailable(true);" << endl;
  }
  pC << "  }" << endl;
  pC << "}" << endl;
  persImplementation.close();

  // DBUS HELPER

  // NM DBUS classes are not part of the namespace
  QString savedNameSpace = nameSpace;
  nameSpace = QString();

  QString dbusHeaderFilename = baseName + "dbus.h";
  QString dbusFileName = baseName + "dbus.cpp";
  QFile dbusHeader( baseDir +  dbusHeaderFilename );
  if ( !dbusHeader.open( QIODevice::WriteOnly ) ) {
    std::cerr << "Can not open '" << qPrintable(dbusHeaderFilename) << "for writing."
              << std::endl;
    return 1;
  }

  QTextStream dH( &dbusHeader );

  inherits = "SettingDbus";

  dH << "// This file is generated by kconfig_compiler from " << QFileInfo(inputFilename).fileName() << "." << endl;
  dH << "// All changes you do to this file will be lost." << endl;

  dH << "#ifndef " << ( !nameSpace.isEmpty() ? nameSpace.toUpper() + '_' : "" )
    << className.toUpper() << "DBUS_H" << endl;
  dH << "#define " << ( !nameSpace.isEmpty() ? nameSpace.toUpper() + '_' : "" )
    << className.toUpper() << "DBUS_H" << endl << endl;

  dH << "#include <nm-setting-" << settingName << ".h>" << endl << endl;

  dH << "#include <kdebug.h>" << endl;
  dH << "#include <kcoreconfigskeleton.h>" << endl;
  dH << "#include \"settingdbus.h\"" << endl;
  dH << "#include \"knm_export.h\"" << endl;

  if ( !nameSpace.isEmpty() )
    dH << "namespace " << nameSpace << " {" << endl << endl;

  dH << "namespace Knm{" << endl;
  dH << "    class " << className << "Setting;" << endl;
  dH << "}" << endl << endl;

  // Class declaration header
  dH << "class " << visibility << className << "Dbus : public " << inherits << endl;
  dH << "{" << endl;
  dH << "  public:" << endl;
  dH << "    " << className << "Dbus(Knm::" << className << "Setting * setting);" << endl;
  dH << "    ~" << className << "Dbus();" << endl;
  dH << "    void fromMap(const QVariantMap&);" << endl;
  dH << "    QVariantMap toMap();" << endl;
  dH << "    QVariantMap toSecretsMap();" << endl;
  dH << "};" << endl;
  if ( !nameSpace.isEmpty() ) dH << "}" << endl << endl;

  dH << "#endif" << endl << endl;

  dbusHeader.close();

  QFile dbusImplementation( baseDir + dbusFileName );
  if ( !dbusImplementation.open( QIODevice::WriteOnly ) ) {
    std::cerr << "Can not open '" << qPrintable(dbusFileName) << "for writing."
              << std::endl;
    return 1;
  }

  QTextStream dC( &dbusImplementation );


  dC << "// This file is generated by kconfig_compiler from " << QFileInfo(inputFilename).fileName() << "." << endl;
  dC << "// All changes you do to this file will be lost." << endl << endl;

  dC << "#include \"" << dbusHeaderFilename << "\"" << endl << endl;
  dC << "#include \"" << headerFileName << "\"" << endl << endl;

  if ( !nameSpace.isEmpty() )
      // using the saved namespace for library classes
    dC << "using namespace " << savedNameSpace << ";" << endl << endl;

  // Constructor
  dC << className << "Dbus::" << className << "Dbus(Knm::" << className << "Setting * setting) : SettingDbus(setting)" << endl;
  dC << "{" << endl;
  dC << "}" << endl << endl;

  // Destructor
  dC << className << "Dbus::~" << className << "Dbus()" << endl;
  dC << "{" << endl;
  dC << "}" << endl << endl;

  // fromMap method
  dC << "void " << className << "Dbus::fromMap(const QVariantMap & map)" << endl;
  dC << "{" << endl;
  dC << "  Knm::" << className << "Setting * setting = static_cast<Knm::" << className << "Setting *>(m_setting);" << endl;
  for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
    if ((*itEntry)->noDbus()) {
        continue;
    }

    QString n = (*itEntry)->name();
    QString t = (*itEntry)->type();

    QString defaultStr;
    if ( !(*itEntry)->defaultValue().isEmpty() )
        defaultStr = (*itEntry)->defaultValue();
    else
        defaultStr = defaultValue( (*itEntry)->type() );

    if ((*itEntry)->secret()) {
        dC << "  // SECRET" << endl;
    }
    dC << "  if (map.contains(" << (*itEntry)->dbusKey() << ")) {" << endl;
    dC << "    setting->" << setFunction(n) << "(map.value(" << (*itEntry)->dbusKey() << ").value<" << cppType(t) << ">());" << endl;
    dC << "  }" << endl;
    //dC << "  setting->" << setFunction(n) << "(m_config->readEntry(\"" << (*itEntry)->key() << "\", " << defaultStr << "));" << endl;
  }
  dC << "}" << endl << endl;

  // toMap method
  dC << "QVariantMap " << className << "Dbus::toMap()" << endl;
  dC << "{" << endl;
  dC << "  QVariantMap map;" << endl;
  dC << "  Knm::" << className << "Setting * setting = static_cast<Knm::" << className << "Setting *>(m_setting);" << endl;
  for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
    if ((*itEntry)->noDbus()) {
        continue;
    }

    QString n = (*itEntry)->name();
    QString t = (*itEntry)->type();

    if ((*itEntry)->secret()) {
        continue;
    }
    if (t == "Enum") {
        const CfgEntry::Choices &choices = (*itEntry)->choices();
        QList<CfgEntry::Choice> chlist = choices.choices;
        if ( chlist.isEmpty() ) {
            // as non-enum
            dC << "  map.insert(" << (*itEntry)->dbusKey() << ", setting->" << getFunction(n) << "());" << endl;
        } else {
            dC << "  switch (setting->" << getFunction(n) << "()) {" << endl;
            foreach (CfgEntry::Choice ch, chlist) {
                dC << "    case Knm::" << className << "Setting::" << enumName(n, choices) << "::" << choices.prefix << ch.name << ":" << endl;
                dC << "      map.insert(" << (*itEntry)->dbusKey() << ", \"" << (ch.dbusValue.isEmpty() ? ch.name : ch.dbusValue) << "\");" << endl;
                dC << "      break;" << endl;
            }
            dC << "  }" << endl;
        }
    } else {
        dC << "  map.insert(" << (*itEntry)->dbusKey() << ", setting->" << getFunction(n) << "());" << endl;
    }
  }
  dC << "  return map;" << endl;
  dC << "}" << endl << endl;

  // toSecretsMap method
  dC << "QVariantMap " << className << "Dbus::toSecretsMap()" << endl;
  dC << "{" << endl;
  dC << "  QVariantMap map;" << endl;
  if (hasSecrets) {
    dC << "  Knm::" << className << "Setting * setting = static_cast<Knm::" << className << "Setting *>(m_setting);" << endl;
  }
  for( itEntry = entries.constBegin(); itEntry != entries.constEnd(); ++itEntry ) {
    if ((*itEntry)->noDbus()) {
      continue;
    }
    QString n = (*itEntry)->name();
    QString t = (*itEntry)->type();

    if (!(*itEntry)->secret()) {
        continue;
    }
    dC << "  map.insert(" << (*itEntry)->dbusKey() << ", setting->" << getFunction(n) << "());" << endl;
  }
  dC << "  return map;" << endl;

  dC << "}" << endl << endl;

  dbusImplementation.close();

  nameSpace = savedNameSpace;

  // clear entries list
  qDeleteAll( entries );
}
