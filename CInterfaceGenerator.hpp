#ifndef CINTERFACEGENERATOR_HPP
#define CINTERFACEGENERATOR_HPP

#include <QString>
#include <QList>

class CNameFeature {
public:
    CNameFeature() { }
    CNameFeature(const QString &newName) { setName(newName); }
    QString name() const { return m_name; }
    QString nameAsIs() const { return m_nameAsIs; }
    QString nameFirstCapital() const;

    void setName(const QString &newName);

private:
    QString m_name;
    QString m_nameAsIs;
};

class CTypeFeature : public CNameFeature {
public:
    CTypeFeature() { }

    QString type() const { return m_type; }
    QString typeForAdaptee() const { return m_typeForAdaptee; }
    QString defaultValue() const { return m_defaultValue; }

    void setTypeFromStr(const QString &type, const QString &tpType);

    bool isPod() const;
    QString formatTypeArgument(bool addName) const;

private:
    QString supposeType(const QString &type, QString tpType) const;
    QString m_type;
    QString m_typeForAdaptee;
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

    Direction direction() const { return m_direction; }

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

    bool isNotifier() const { return m_isNotifier; }
    void setNotifierFlag(bool isNotifier);

private:
    bool m_isNotifier;

};

class CInterfaceProperty : public CTypeFeature {
public:
    CInterfaceProperty() : CTypeFeature(), m_notifier(0), m_immutable(false), m_unchangeable(false) { }

    CInterfaceSignal *notifier() const { return m_notifier; }
    void setNotifier(CInterfaceSignal *notifier);
    bool isImmutable() const { return m_immutable || m_unchangeable; }
    bool isUnchangeable() const { return m_unchangeable; }
    void setImmutable(bool newImmutable);
    void setUnchangeable(bool newUnchangeable);

    QString dbusGetter() const;

private:
    CInterfaceSignal *m_notifier;
    bool m_immutable;
    bool m_unchangeable;

};

class CInterfaceMethod : public CNameFeature, public CArgumentsFeature  {
public:
    CInterfaceMethod(const QString &name);
    QString callbackType() const { return nameAsIs() + QLatin1String("Callback"); }
    QString callbackMember() const { return name() + QLatin1String("CB"); }
    QString callbackRetType() const { return m_callbackRetType; }

    QList<uint> outputArgsIndices() const { return m_outputArgsIndices; }

    void prepare();

private:
    QString m_callbackRetType;
    QList<uint> m_outputArgsIndices;

};

class CInterfaceGenerator
{
public:
    enum class SpecFormat {
        Invalid,
        Classic,
    };
    enum InterfaceType {
        InterfaceTypeInvalid,
        InterfaceTypeChannel,
        InterfaceTypeConnection,
        InterfaceTypeProtocol
    };

    enum InterfaceSubType {
        InterfaceSubTypeInvalid,
        InterfaceSubTypeBaseClass,
        InterfaceSubTypeType,
        InterfaceSubTypeInterface
    };

    CInterfaceGenerator();

    SpecFormat specFormat() const;
    bool isValid() const;

    QString className() const;
    QString parentClassPrefix() const;
    QString classPtr() const;
    QString interfaceSubclass() const;
    QString classBaseType() const;
    QString interfaceTypeShort() const;
    QString subTypeStr() const;
    QString docGroup() const;

    QString interfaceTpDefinition() const;

    static InterfaceType strToType(const QString &str);

    QString interfaceName() const { return m_name; }
    void setFullName(const QString &name);

    QString shortName() const;
    QString fullName() const { return m_fullName; }

    QString node() const { return m_node; }
    QString nodeName() const { return m_nodeName; }
    void setNode(const QString &node);

    void setType(const QString &classBaseType);
    void setSubType(InterfaceSubType subType);
    void setEmitPropertiesChangedSignal(bool enable);

    void prepare();
    QString generateHeaderInterface() const;
    QString generateHeaderAdaptee() const;

    QString generateImplementationAdaptee() const;
    QString generateImplementationPrivate() const;
    QString generateImplementationInterface() const;

    QString generateImplementations() const;

    QString getServiceAdaptor() const;

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

    QString m_adapteeParentMember;
    InterfaceType m_type;
    InterfaceSubType m_subType;
    QString m_node;
    QString m_nodeName;
    QString m_name;
    QString m_fullName;
    SpecFormat m_specFormat = SpecFormat::Invalid;
    int m_mutablePropertiesCount;
    int m_immutablePropertiesCount;
    bool m_emitPropertiesChangedSignal;

};

#endif // CINTERFACEGENERATOR_HPP
