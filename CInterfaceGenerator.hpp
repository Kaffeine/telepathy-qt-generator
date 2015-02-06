#ifndef CINTERFACEGENERATOR_HPP
#define CINTERFACEGENERATOR_HPP

#include <QString>
#include <QList>

class CNameFeature {
public:
    CNameFeature() { }
    CNameFeature(const QString &newName) { setName(newName); }
    CNameFeature(const CNameFeature &another) : m_name(another.name()) { }
    inline QString name() const { return m_name; }
    inline QString nameAsIs() const { return m_nameAsIs; }
    QString nameFirstCapital() const;

    void setName(const QString &newName);

private:
    QString m_name;
    QString m_nameAsIs;
};

class CTypeFeature : public CNameFeature {
public:
    CTypeFeature() { }
    CTypeFeature(const CTypeFeature &another) : CNameFeature(another), m_type(another.m_type) { }

    inline QString type() const { return m_type; }
    inline QString typeSimplified() const { return m_typeSimplified; }
    inline QString defaultValue() const { return m_defaultValue; }

    void setTypeFromStr(const QString &type, const QString &tpType);

    bool isPod() const;
    QString formatTypeArgument(bool addName) const;

private:
    QString supposeType(const QString &type, QString tpType) const;
    QString m_type;
    QString m_typeSimplified;
    QString m_defaultValue;

};

class CMethodArgument : public CTypeFeature {
public:
    enum Direction {
        Input,
        Output,
        Invalid
    };

    CMethodArgument() : CTypeFeature(), m_direction(Invalid) { }
    CMethodArgument(const CMethodArgument &arg) : CTypeFeature(arg), m_direction(arg.m_direction) { }

    inline Direction direction() const { return m_direction; }

    void setDirection(const QString &directionStr);

    QString formatArgument(bool addName) const;

private:
    Direction m_direction;
};

class CArgumentsFeature
{
public:
    QList<CMethodArgument> arguments;
    bool isSimple() const;

};

class CInterfaceSignal : public CNameFeature, public CArgumentsFeature {
public:
    CInterfaceSignal(const QString &name);

    inline bool isNotifier() const { return m_isNotifier; }
    void setNotifierFlag(bool isNotifier);

private:
    bool m_isNotifier;

};

class CInterfaceProperty : public CTypeFeature {
public:
    CInterfaceProperty() : CTypeFeature(), m_notifier(0), m_immutable(false) { }
    CInterfaceProperty(const CInterfaceProperty &prop) : CTypeFeature(prop), m_notifier(prop.m_notifier), m_immutable(prop.m_immutable) { }

    inline CInterfaceSignal *notifier() const { return m_notifier; }
    void setNotifier(CInterfaceSignal *notifier);
    inline bool isImmutable() const { return m_immutable; }
    void setImmutable(bool newImmutable);

private:
    CInterfaceSignal *m_notifier;
    bool m_immutable;

};

class CInterfaceMethod : public CNameFeature, public CArgumentsFeature  {
public:
    CInterfaceMethod(const QString &name);
    inline QString callbackType() const { return nameAsIs() + QLatin1String("Callback"); }
    inline QString callbackMember() const { return name() + QLatin1String("CB"); }
    inline QString callbackRetType() const { return m_callbackRetType; }

    QList<uint> outputArgsIndices() const { return m_outputArgsIndices; }

    void prepare();

private:
    QString m_callbackRetType;
    QList<uint> m_outputArgsIndices;

};

class CInterfaceGenerator
{
public:
    enum InterfaceType {
        InterfaceTypeChannel,
        InterfaceTypeConnection,
        InterfaceTypeProtocol,
        InterfaceTypeInvalid
    };

    CInterfaceGenerator();

    QString className() const;
    QString classPtr() const;
    QString interfaceSubclass() const;
    QString classBaseType() const;
    QString interfaceTypeShort() const;

    QString interfaceTpDefinition() const;

    static InterfaceType strToType(const QString &str);

    inline QString interfaceName() const { return m_name; }
    void setName(const QString &name);

    QString shortName() const;

    inline QString node() const { return m_node; }
    inline QString nodeName() const { return m_nodeName; }
    void setNode(const QString &node);

    void setType(const QString &classBaseType);

    void prepare();
    QString generateHeaderInterface() const;
    QString generateHeaderAdaptee() const;

    QString generateImplementationAdaptee() const;
    QString generateImplementationPrivate() const;
    QString generateImplementationInterface() const;

    QString generateImplementations() const;

    QList<CInterfaceSignal*> m_signals;
    QList<CInterfaceProperty*> m_properties;
    QList<CInterfaceMethod*> m_methods;
private:

    QString generateImmutablePropertiesListHelper(const int creatorSpacing, bool names, bool signatures) const;
    QString generatePrivateConstructorPropertiesList(const int creatorSpacing) const;
    QString generateMethodCallbackAndDeclaration(const CInterfaceMethod *method) const;
    QString formatArguments(const CArgumentsFeature *argumentsClass, bool argName, bool hideOutputArguments = false, bool addType = true) const;
    QString formatArgument(const CMethodArgument &arg, bool addName) const;
    QString formatInvokeMethodArguments(const CArgumentsFeature *argumentsClass) const;
    InterfaceType m_type;
    QString m_node;
    QString m_nodeName;
    QString m_name;

    int m_mutablePropertiesCount;
    int m_immutablePropertiesCount;

};

#endif // CINTERFACEGENERATOR_HPP
