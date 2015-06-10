#include "CInterfaceGenerator.hpp"

#include <QStringList>
#include <QDebug>

static const QString spacing = QLatin1String("    ");

static bool compatibleWithQt4 = true;

QString formatName(QString name)
{
    static const QStringList abs = QStringList() << QLatin1String("SASL") << QLatin1String("URI") << QLatin1String("MIME");

    int index = 0;
    foreach (const QString &abb, abs) {
        while ((index = name.indexOf(abb)) >= 0) {
            name.replace(index, abb.size(), abb.toLower());
            if (index > 0) {
                name[index] = name.at(index).toUpper();
            }
        }
    }

    while ((index = name.indexOf(QLatin1Char('_'))) > 0) {
        if ((name.length() < index + 1) || (!name.at(index + 1).isLetter())) {
            break;
        }
        name[index + 1] = name.at(index + 1).toUpper();
        name.remove(index, 1);
    }

    return name;
}

void CNameFeature::setName(const QString &newName)
{
    m_nameAsIs = newName;
    m_name = formatName(newName);

    m_name[0] = m_name.at(0).toLower();
}

QString CNameFeature::nameFirstCapital() const
{
    QString result = m_name;
    result[0] = result.at(0).toUpper();
    return result;
}

void CTypeFeature::setTypeFromStr(const QString &type, const QString &tpType)
{
    if (type == QLatin1String("b")) {
        m_type = QLatin1String("bool");
        m_defaultValue = QLatin1String("false");
    } else if (type == QLatin1String("y")) {
        m_type = QLatin1String("uchar");
    } else if (type == QLatin1String("n")) {
        m_type = QLatin1String("short");
    } else if (type == QLatin1String("q")) {
        m_type = QLatin1String("ushort");
    } else if (type == QLatin1String("i")) {
        m_type = QLatin1String("int");
    } else if (type == QLatin1String("u")) {
        m_typeSimplified = QLatin1String("uint");

        if (!tpType.endsWith(QLatin1String("Flags"))) {
            m_type = m_typeSimplified;
        }

        if (tpType == QLatin1String("Contact_List_State")) {
            m_defaultValue = QLatin1String("ContactListStateNone");
        } else /*if (tpType == QLatin1String("Contact_Info_Flags"))*/ {
            m_defaultValue = QLatin1String("0");
        }
    } else if (type == QLatin1String("x")) {
        m_type = QLatin1String("qlonglong");
    } else if (type == QLatin1String("t")) {
        m_type = QLatin1String("qulonglong");
    } else if (type == QLatin1String("d")) {
        m_type = QLatin1String("double");
    } else if (type == QLatin1String("s")) {
        m_type = QLatin1String("QString");
    } else if (type == QLatin1String("as")) {
        m_type = QLatin1String("QStringList");
    } else if (type == QLatin1String("au")) {
        m_type = QLatin1String("Tp::UIntList");
    } else if (type == QLatin1String("ay")) {
        m_type = QLatin1String("QByteArray");
    } else if (type == QLatin1String("o")) {
        m_type = QLatin1String("QDBusObjectPath");
    } else if (type == QLatin1String("ao")) {
        m_type = QLatin1String("Tp::ObjectPathList");
    } else if (type == QLatin1String("g")) {
        m_type = QLatin1String("QDBusSignature");
    } else if (type == QLatin1String("ag")) {
        m_type = QLatin1String("Tp::SignatureList");
    } else if (type == QLatin1String("v")) {
        m_type = QLatin1String("QDBusVariant");
    } else if (type == QLatin1String("av")) {
        m_type = QLatin1String("QVariantList");
    } else if (type == QLatin1String("aav")) {
        m_type = QLatin1String("Tp::VariantListList");
    } else if (type == QLatin1String("a{sv}")) {
        m_type = QLatin1String("QVariantMap");
    } else {
        if (tpType == QLatin1String("Field_Spec[]") && type == QLatin1String("a(sasuu)")) {
            m_type = QLatin1String("Tp::FieldSpecs");
        }
    }

    // Not really correct. See https://bugs.freedesktop.org/show_bug.cgi?id=21690.
    if (tpType == QLatin1String("Unix_Timestamp64")) {
        m_type = QLatin1String("QDateTime");
    }

    if (m_defaultValue.isEmpty() && isPod()) {
        m_defaultValue = QLatin1String("0");
    }

    if (m_type.isEmpty()) {
        m_type = supposeType(type, tpType);
    }

    if (m_typeSimplified.isEmpty()) {
        m_typeSimplified = m_type;
    }
}

bool CTypeFeature::isPod() const
{
    static const QStringList podTypes = QStringList()
            << QLatin1String("bool")
            << QLatin1String("uchar")
            << QLatin1String("short")
            << QLatin1String("ushort")
            << QLatin1String("int")
            << QLatin1String("uint")
            << QLatin1String("qlonglong")
            << QLatin1String("qulonglong")
            << QLatin1String("double");

    return podTypes.contains(m_type);
}

QString CTypeFeature::formatTypeArgument(bool addName) const
{
    if (isPod()) {
        if (addName) {
            return QString(QLatin1String("%1 %2")).arg(m_type).arg(name());
        } else {
            return m_type;
        }
    } else {
        if (addName) {
            return QString(QLatin1String("const %1 &%2")).arg(m_type).arg(name());
        } else {
            return QString(QLatin1String("const %1 &")).arg(m_type);
        }
    }

    return QString();
}

QString CTypeFeature::supposeType(const QString &type, QString tpType) const
{
    Q_UNUSED(type);

    if (tpType.isEmpty()) {
        qDebug() << "Can't suppose type: There is no tp-type tag. Type:" << type;
    }

    QString suffix;
    if (tpType.endsWith(QLatin1String("[]"))) {
        tpType.chop(2);
        suffix = QLatin1String("List");
    }

    return QLatin1String("Tp::") + tpType.remove(QLatin1Char('_')) + suffix;
}

void CMethodArgument::setDirection(const QString &directionStr)
{
    if (directionStr == QLatin1String("in")) {
        m_direction = Input;
    } else if (directionStr == QLatin1String("out")) {
        m_direction = Output;
    } else {
        m_direction = Invalid;
    }
}

QString CMethodArgument::formatArgument(bool addName) const
{
    if (m_direction == Output) {
        if (addName) {
            return QString(QLatin1String("%1 &%2")).arg(type()).arg(name());
        } else {
            return QString(QLatin1String("%1 &")).arg(type());
        }
    } else {
        return formatTypeArgument(addName);
    }
}

bool CArgumentsFeature::isSimple() const
{
    return arguments.isEmpty() || ((arguments.count() == 1) && arguments.first().direction() == CMethodArgument::Output);
}

CInterfaceSignal::CInterfaceSignal(const QString &name) :
    CNameFeature(name),
    m_isNotifier(false)
{
}

void CInterfaceSignal::setNotifierFlag(bool isNotifier)
{
    m_isNotifier = isNotifier;
}

void CInterfaceProperty::setNotifier(CInterfaceSignal *notifier)
{
    m_notifier = notifier;
}

void CInterfaceProperty::setImmutable(bool newImmutable)
{
    m_immutable = newImmutable;
}

void CInterfaceProperty::setUnchangeable(bool newUnchangeable)
{
    m_unchangeable = newUnchangeable;
}

CInterfaceMethod::CInterfaceMethod(const QString &name) :
    CNameFeature(name)
{
}

void CInterfaceMethod::prepare()
{
    m_outputArgsIndices.clear();

    for (int i = 0; i < arguments.count(); ++i) {
        if (arguments.at(i).direction() == CMethodArgument::Output) {
            m_outputArgsIndices.append(i);
        }
    }

    QString retType = QLatin1String("void");
    if (m_outputArgsIndices.count() == 1) {
        retType = arguments.at(m_outputArgsIndices.first()).type();
    }

    m_callbackRetType = retType;
}

CInterfaceGenerator::CInterfaceGenerator() :
    m_type(InterfaceTypeInvalid),
    m_mutablePropertiesCount(0),
    m_immutablePropertiesCount(0),
    m_emitPropertiesChangedSignal(false)
{
}

QString CInterfaceGenerator::className() const
{
    QString classSuffix;

    switch (m_subType) {
    case InterfaceSubTypeBaseClass:
        return QString(QLatin1String("Base%1")).arg(classBaseType());
        break;
    case InterfaceSubTypeType:
        classSuffix = QLatin1String("Type");
        break;
    case InterfaceSubTypeInterface:
        classSuffix = QLatin1String("Interface");
        break;
    default:
        break;
    }

    return QString(QLatin1String("Base%1%2%3")).arg(classBaseType()).arg(nodeName()).arg(classSuffix);
}

QString CInterfaceGenerator::parentClassPrefix() const
{
    if (m_subType == InterfaceSubTypeBaseClass) {
        return QLatin1String("Base");
    }
    return classBaseType();
}

QString CInterfaceGenerator::classPtr() const
{
    return className() + QLatin1String("Ptr");
}

QString CInterfaceGenerator::interfaceSubclass() const
{
    return className() + QLatin1String("Subclass");
}

QString CInterfaceGenerator::classBaseType() const
{
    switch (m_type) {
    case InterfaceTypeChannel:
        return QLatin1String("Channel");
    case InterfaceTypeConnection:
        return QLatin1String("Connection");
    case InterfaceTypeProtocol:
        return QLatin1String("Protocol");
    default:
        return QString();
    }
}

QString CInterfaceGenerator::interfaceTypeShort() const
{
    switch (m_type) {
    case InterfaceTypeChannel:
        return QLatin1String("Chan");
    case InterfaceTypeConnection:
        return QLatin1String("Conn");
    case InterfaceTypeProtocol:
        return QLatin1String("Proto");
    default:
        return QString();
    }
}

QString CInterfaceGenerator::subTypeStr() const
{
    switch (m_subType) {
    case InterfaceSubTypeType:
        return QLatin1String("Type");
        break;
    case InterfaceSubTypeInterface:
        return QLatin1String("Interface");
        break;
    default:
        return QString();
    }
}

QString CInterfaceGenerator::interfaceTpDefinition() const
{
    return QLatin1String("TP_QT_IFACE_") + node().toUpper();
}

CInterfaceGenerator::InterfaceType CInterfaceGenerator::strToType(const QString &str)
{
    if (str == QLatin1String("Channel"))
        return InterfaceTypeChannel;
    else if (str == QLatin1String("Connection"))
        return InterfaceTypeConnection;
    else if (str == QLatin1String("Protocol"))
        return InterfaceTypeProtocol;

    return InterfaceTypeInvalid;
}

void CInterfaceGenerator::setFullName(const QString &name)
{
    QStringList nameParts = name.split(QLatin1Char('.'));
    m_name = nameParts.last();

    nameParts = nameParts.mid(3);
    m_fullName = nameParts.join(QLatin1Char('.'));
}

QString CInterfaceGenerator::shortName() const
{
    QChar middleChar;

    switch (m_subType) {
    case InterfaceSubTypeBaseClass:
        return classBaseType();
    case InterfaceSubTypeType:
        middleChar = QLatin1Char('T');
        break;
    case InterfaceSubTypeInterface:
        middleChar = QLatin1Char('I');
        break;
    default:
        return QString();
        break;
    }
    return QString(QLatin1String("%1.%2.%3")).arg(interfaceTypeShort()).arg(middleChar).arg(interfaceName());
}

void CInterfaceGenerator::setNode(const QString &node)
{
    m_node = node.mid(1); // Skip '/'

    QStringList nameParts = m_node.split(QLatin1Char('_'));

    if (nameParts.count() == 1) {
        m_nodeName = m_node;
    } else {
        nameParts = nameParts.mid(2);
        m_nodeName = nameParts.join(QLatin1String(""));
    }
}

void CInterfaceGenerator::setType(const QString &typeStr)
{
    m_type = strToType(typeStr);
}

void CInterfaceGenerator::setSubType(CInterfaceGenerator::InterfaceSubType subType)
{
    m_subType = subType;
}

void CInterfaceGenerator::setEmitPropertiesChangedSignal(bool enable)
{
    m_emitPropertiesChangedSignal = enable;
}

void CInterfaceGenerator::prepare()
{
    m_mutablePropertiesCount = 0;
    m_immutablePropertiesCount = 0;

    for (int i = 0; i < m_properties.count(); ++i) {
        if (m_properties.at(i)->isImmutable()) {
            ++m_immutablePropertiesCount;
        } else {
            ++m_mutablePropertiesCount;
        }

        for (int j = 0; j < m_signals.count(); ++j) {
            if (m_signals.at(j)->name() == m_properties.at(i)->name() + QLatin1String("Changed")) {
                m_properties[i]->setNotifier(m_signals.at(j));
                m_signals[j]->setNotifierFlag(true);
                break;
            }
        }
    }

    for (int i = 0; i < m_methods.count(); ++i) {
        m_methods.at(i)->prepare();
    }

    if (m_subType == InterfaceSubTypeBaseClass) {
        m_adapteeParentMember = QLatin1Char('m') + nodeName();
    } else {
        m_adapteeParentMember = QLatin1String("mInterface");
    }
}

QString CInterfaceGenerator::generateImmutablePropertiesListHelper(const int creatorSpacing, bool names, bool signatures) const
{
    QString result;

    QString creatorSpacingStr;

    if (creatorSpacing > 0) {
        creatorSpacingStr = QString(creatorSpacing, QLatin1Char(' '));
    }

    for (int i = 0; i < m_properties.count(); ++i) {
        if (!m_properties.at(i)->isImmutable()) {
            continue;
        }

        if (!result.isEmpty()) {
            result += QLatin1Char(',');
            if (creatorSpacing > 0) {
                result += QLatin1Char('\n') + creatorSpacingStr;
            } else {
                result += QLatin1Char(' ');
            }
        }

        if (signatures) {
            result += m_properties.at(i)->formatTypeArgument(names);
        } else {
            // Assume it as names only
            result += m_properties.at(i)->name();
        }
    }

    return result;
}

QString CInterfaceGenerator::generatePrivateConstructorPropertiesList(const int creatorSpacing) const
{
    QString result;
    QString creatorSpacingStr(creatorSpacing, QLatin1Char(' '));

    for (int i = 0; i < m_properties.count(); ++i) {
        CInterfaceProperty *prop = m_properties.at(i);
        QString propValue;

        if (prop->isImmutable()) {
            propValue = prop->name();
        } else {
            if (prop->isPod()) {
                propValue = prop->defaultValue();
            } else {
                continue;
            }
        }

        result += QString(QLatin1String("%1(%2),\n")).arg(prop->name()).arg(propValue);
        result += creatorSpacingStr;
    }

    return result;
}

QString CInterfaceGenerator::generateHeaderInterface() const
{
    QString result;

    result += QLatin1String("class TP_QT_EXPORT ") + className();
    result += QString(QLatin1String(" : public Abstract%1Interface\n{\n")).arg(parentClassPrefix());
    result += QString(QLatin1String("%1Q_OBJECT\n")).arg(spacing);
    result += QString(QLatin1String("%1Q_DISABLE_COPY(%2)\n\n")).arg(spacing).arg(className());

    result += QLatin1String("public:\n");

    QString creatorLine;
    // Create
    creatorLine = spacing + QString(QLatin1String("static %1 create(")).arg(classPtr());
    result += creatorLine;
    result += generateImmutablePropertiesListHelper(creatorLine.size(), /* names */ true, /* signature */ true);
    result += QLatin1String(")\n");
    result += spacing + QLatin1String("{\n");
    creatorLine = spacing + spacing + QString(QLatin1String("return %1(new %2(")).arg(classPtr()).arg(className());
    result += creatorLine;
    result += generateImmutablePropertiesListHelper(creatorLine.size(), /* names */ true, /* signature */ false);
    result += QLatin1String("));\n");
    result += spacing + QLatin1String("}\n");

    // Subclass template
    result += spacing + QString(QLatin1String("template<typename %1>\n")).arg(interfaceSubclass());

    creatorLine = spacing + QString(QLatin1String("static SharedPtr<%1> create(")).arg(interfaceSubclass());
    result += creatorLine;
    result += generateImmutablePropertiesListHelper(creatorLine.size(), /* names */ true, /* signature */ true);
    result += QLatin1String(")\n");
    result += spacing + QLatin1String("{\n");
    result += spacing + spacing + QString(QLatin1String("return SharedPtr<%1>(\n")).arg(interfaceSubclass());
//
    creatorLine = spacing + spacing + spacing + spacing;
    creatorLine += QString(QLatin1String("new %1(")).arg(interfaceSubclass());
    result += creatorLine;
    result += generateImmutablePropertiesListHelper(creatorLine.size(), /* names */ true, /* signature */ false);
    result += QLatin1String("));\n");
    result += spacing + QLatin1String("}\n");

    result += QLatin1Char('\n');
    result += spacing + QString(QLatin1String("virtual ~%1();\n")).arg(className());
    result += QLatin1Char('\n');
    result += spacing + QLatin1String("QVariantMap immutableProperties() const;\n");
    result += QLatin1Char('\n');

    // Immutable properties
    if (m_immutablePropertiesCount) {
        foreach (const CInterfaceProperty *prop, m_properties) {
            if (!prop->isImmutable()) {
                continue;
            }

            result += spacing + QString(QLatin1String("%1 %2() const;\n")).arg(prop->type()).arg(prop->name());
        }
        if (!result.endsWith(QLatin1String("\n\n"))) {
            result += QLatin1Char('\n');
        }
    }

    // Mutable properties
    if (m_mutablePropertiesCount) {
        foreach (const CInterfaceProperty *prop, m_properties) {
            if (prop->isImmutable()) {
                continue;
            }

            result += spacing + QString(QLatin1String("%1 %2() const;\n")).arg(prop->type()).arg(prop->name());
            if (prop->notifier()) {
                result += spacing + QString(QLatin1String("void set%1(%2);\n")).arg(prop->nameFirstCapital()).arg(formatArguments(prop->notifier(), /* addName*/ true));
            } else {
                result += spacing + QString(QLatin1String("void set%1(%2);\n")).arg(prop->nameFirstCapital()).arg(prop->formatTypeArgument(/* addName*/ true));
            }
            result += QLatin1Char('\n');
        }
    }

    // Methods
    foreach (const CInterfaceMethod *method, m_methods) {
        result += generateMethodCallbackAndDeclaration(method);
        result += QLatin1Char('\n');
    }

    // Signals (not notifiers)
    foreach (const CInterfaceSignal *signal, m_signals) {
        if (signal->isNotifier()) {
            continue;
        }

        result += spacing + QString(QLatin1String("void %1(%2);\n")).arg(signal->name()).arg(formatArguments(signal, /* addName*/ true));
    }

    if (!m_signals.isEmpty()) {
        result += QLatin1Char('\n');
    }

    result += QLatin1String("protected:\n");
    creatorLine = spacing + className() + QLatin1Char('(');
    result += creatorLine;
    result += generateImmutablePropertiesListHelper(creatorLine.size(), /* names */ true, /* signature */ true);
    result += QLatin1String(");\n");

    result += QLatin1Char('\n');
    result += QLatin1String("private:\n");

    result += spacing + QLatin1String("void createAdaptor();\n\n");
    result += spacing + QLatin1String("class Adaptee;\n");
    result += spacing + QLatin1String("friend class Adaptee;\n");
    result += spacing + QLatin1String("struct Private;\n");
    result += spacing + QLatin1String("friend struct Private;\n");
    result += spacing + QLatin1String("Private *mPriv;\n");
    result += QLatin1String("};\n\n");

    return result;
}

QString CInterfaceGenerator::generateHeaderAdaptee() const
{
    QString result;

    result += QString(QLatin1String("class TP_QT_NO_EXPORT %1::Adaptee : public QObject\n")).arg(className());

    result += QLatin1String("{\n");
    result += spacing + QLatin1String("Q_OBJECT\n");

    foreach (const CInterfaceProperty *prop, m_properties) {
        result += spacing + QString(QLatin1String("Q_PROPERTY(%1 %2 READ %2)\n")).arg(prop->typeSimplified()).arg(prop->name());
    }

    result += QLatin1Char('\n');
    result += QLatin1String("public:\n");
    result += spacing + QString(QLatin1String("Adaptee(%1 *interface);\n")).arg(className());
    result += spacing + QLatin1String("~Adaptee();\n");

    result += QLatin1Char('\n');

    if (!m_properties.isEmpty()) {
        foreach (const CInterfaceProperty *prop, m_properties) {
            result += spacing + QString(QLatin1String("%1 %2() const;\n")).arg(prop->typeSimplified()).arg(prop->name());
        }

        result += QLatin1Char('\n');
    }

    if (!m_methods.isEmpty()) {
        result += QLatin1String("private Q_SLOTS:\n");


        foreach (const CInterfaceMethod *method, m_methods) {
            result += spacing + QString(QLatin1String("void %1(%2\n")).arg(method->name())
                    .arg(method->isSimple() ? QString() : formatArguments(method, /* name */ true, /* hideOutput */ true) + QLatin1String(","));
            result += spacing + spacing + spacing + QString(QLatin1String("const %1::%2ContextPtr &context);\n")).arg(getServiceAdaptor()).arg(method->nameAsIs());
        }

        result += QLatin1Char('\n');
    }

    if (!m_signals.isEmpty()) {
        result += QLatin1String("Q_SIGNALS:\n");
        foreach (const CInterfaceSignal *sig, m_signals) {
            result += spacing + QString(QLatin1String("void %1(%2);\n")).arg(sig->name()).arg(formatArguments(sig, /* name */ true));
        }

        result += QLatin1Char('\n');
    }

    result += QLatin1String("private:\n");
    result += spacing + QString(QLatin1String("%1 *%2;\n")).arg(className()).arg(m_adapteeParentMember);

    result += QLatin1String("};\n");

    return result;
}

QString CInterfaceGenerator::generateImplementations() const
{
    QString result;

    result += QLatin1String("// ") + shortName() + QLatin1Char('\n');
    result += QLatin1String("// The ") + className() + QLatin1String(" code is fully or partially generated by the TelepathyQt-Generator.\n");

    result += generateImplementationPrivate();
    result += generateImplementationAdaptee();
    result += generateImplementationInterface();

    return result;
}

QString CInterfaceGenerator::getServiceAdaptor() const
{
    return QString(QLatin1String("Tp::Service::%1%2%3Adaptor")).arg(parentClassPrefix()).arg(subTypeStr()).arg(nodeName());
}

QString CInterfaceGenerator::generateImplementationAdaptee() const
{
    QString result;

    const QString adapteeClassName = className() + QLatin1String("::Adaptee");

    result += QString(QLatin1String("%1::Adaptee(%2 *interface)\n")).arg(adapteeClassName).arg(className());
    result += spacing + QLatin1String(": QObject(interface),\n");
    result += spacing + QString(QLatin1String("  %1(interface)\n")).arg(m_adapteeParentMember);
    result += QLatin1String("{\n}\n\n");

    result += QString(QLatin1String("%1::~Adaptee()\n")).arg(adapteeClassName);
    result += QLatin1String("{\n}\n\n");

    // Properties
    foreach (const CInterfaceProperty *prop, m_properties) {
        result += QString(QLatin1String("%1 %2::%3() const\n")).arg(prop->typeSimplified()).arg(adapteeClassName).arg(prop->name());
        result += QLatin1String("{\n");
        result += spacing + QString(QLatin1String("return %1->%2();\n")).arg(m_adapteeParentMember).arg(prop->name());
        result += QLatin1String("}\n");
        result += QLatin1Char('\n');
    }

    // Methods
    foreach (const CInterfaceMethod *method, m_methods) {
        result += QString(QLatin1String("void %1::%2(%3\n")).arg(adapteeClassName).arg(method->name())
                .arg(method->isSimple() ? QString() : formatArguments(method, /* name */ true, /* hideOutput */ true) + QLatin1String(","));
        result += spacing + spacing + QString(QLatin1String("const %1::%2ContextPtr &context)\n")).arg(getServiceAdaptor()).arg(method->nameAsIs());

        result += QLatin1String("{\n");

        result += spacing + QString(QLatin1String("debug() << \"%1::%2\";\n")).arg(adapteeClassName).arg(method->name());
        result += spacing + QLatin1String("DBusError error;\n");

        QStringList outputVarNames;

        foreach (int outputArgumentIndex, method->outputArgsIndices()) {
            outputVarNames.append(method->arguments.at(outputArgumentIndex).name());
        }

        if (method->outputArgsIndices().count() == 1) {
            result += spacing + QString(QLatin1String("%1 %2 = ")).arg(method->callbackRetType()).arg(outputVarNames.first());
        } else {
            if (method->outputArgsIndices().count() > 1) {
                foreach (int outputArgumentIndex, method->outputArgsIndices()) {
                    const CMethodArgument &argument = method->arguments.at(outputArgumentIndex);
                    result += spacing + QString(QLatin1String("%1 %2;\n")).arg(argument.type()).arg(argument.name());
                }

                result += QLatin1Char('\n');
            }
            result += spacing;
        }

        result += m_adapteeParentMember + QString(QLatin1String("->%1(%2&error);\n")).arg(method->name())
                .arg(method->isSimple() ? QString() : formatArguments(method, /* argName */ true,
                                                                      /* hideOutputArguments */ (method->outputArgsIndices().count() == 1),
                                                                      /* addType */ false) + QLatin1String(", "));

        result += spacing + QLatin1String("if (error.isValid()) {\n");
        result += spacing + spacing + QLatin1String("context->setFinishedWithError(error.name(), error.message());\n");
        result += spacing + spacing + QLatin1String("return;\n");
        result += spacing + QLatin1String("}\n");

        result += spacing + QString(QLatin1String("context->setFinished(%1);\n")).arg(outputVarNames.join(QLatin1String(", ")));

        result += QLatin1String("}\n");
        result += QLatin1Char('\n');
    }

    return result;
}

QString CInterfaceGenerator::generateImplementationPrivate() const
{
    QString result;

    // Private
    result += QString(QLatin1String("struct TP_QT_NO_EXPORT %1::Private {\n")).arg(className());

    if (m_immutablePropertiesCount) {
        result += spacing + QString(QLatin1String("Private(%1 *parent,\n")).arg(className());
        static const int privateSpacing = spacing.size() + QString(QLatin1String("Private(")).size();

        result += QString(privateSpacing, QLatin1Char(' '));

        result += generateImmutablePropertiesListHelper(privateSpacing, /* names */ true, /* signature */ true);
        result += QLatin1String(")\n");
    } else {
        result += spacing + QString(QLatin1String("Private(%1 *parent)\n")).arg(className());
    }

    QString creatorLine;

    creatorLine = spacing + spacing + QLatin1String(": ");
    result += creatorLine;
    result += generatePrivateConstructorPropertiesList(creatorLine.size());
    result += QString(QLatin1String("adaptee(new %1::Adaptee(parent))\n")).arg(className());

    result += spacing + QLatin1String("{\n");
    result += spacing + QLatin1String("}\n");

    result += QLatin1Char('\n');

    // Private members
    foreach (const CInterfaceProperty *prop, m_properties) {
        result += spacing + QString(QLatin1String("%1 %2;\n")).arg(prop->type()).arg(prop->name());
    }

    // Methods
    foreach (const CInterfaceMethod *method, m_methods) {
        result += spacing + QString(QLatin1String("%1 %2;\n")).arg(method->callbackType()).arg(method->callbackMember());
    }

    result += spacing + QString(QLatin1String("%1::Adaptee *adaptee;\n")).arg(className());
    result += QLatin1String("};\n");
    result += QLatin1Char('\n');

    return result;
}

QString CInterfaceGenerator::generateImplementationInterface() const
{
    QString result;

    QString creatorLine;
    QString creatorSpacingStr;

    static const QLatin1String commentHeader = QLatin1String(
                "/**\n"
                " * \\class %1\n"
                " * \\ingroup servicecm\n"
                " * \\headerfile TelepathyQt/base-%2.h <TelepathyQt/Base%3>\n"
                " *\n"
                " * \\brief Base class for implementations of %4\n"
                " */\n\n");

    static const QLatin1String commentClassConstructor = QLatin1String(
                "/**\n"
                " * Class constructor.\n"
                " */\n");

    static const QLatin1String commentClassDestructor = QLatin1String(
                "/**\n"
                " * Class destructor.\n"
                " */\n");

    static const QLatin1String commentMethodImmutableProperties = QLatin1String(
                "/**\n"
                " * Return the immutable properties of this interface.\n"
                " *\n"
                " * Immutable properties cannot change after the interface has been registered\n"
                " * on a service on the bus with registerInterface().\n"
                " *\n"
                " * \\return The immutable properties of this interface.\n"
                " */\n");

    result += QString(commentHeader).arg(className()).arg(classBaseType().toLower()).arg(classBaseType()).arg(fullName());

    // Interface Constructor
    result += commentClassConstructor;
    creatorLine = QString(QLatin1String("%1::%1(")).arg(className());
    result += creatorLine;
    result += generateImmutablePropertiesListHelper(creatorLine.size(), /* names */ true, /* signatures */ true);
    result += QLatin1String(")\n");
    creatorLine = spacing + QLatin1String(": ");
    creatorSpacingStr = QString(creatorLine.size(), QLatin1Char(' '));
    result += creatorLine;

    result += QString(QLatin1String("Abstract%1Interface(%2),\n")).arg(parentClassPrefix()).arg(interfaceTpDefinition());

    if (m_immutablePropertiesCount) {
        result += creatorSpacingStr;
        result += QString(QLatin1String("mPriv(new Private(this, %1))\n")).arg(generateImmutablePropertiesListHelper(-1 /* mean no-new-lines */, /* names */ true, /* signatures */ false));
    } else {
        result += creatorSpacingStr + QLatin1String("mPriv(new Private(this))\n");
    }

    result += QLatin1String("{\n");
    result += QLatin1String("}\n");
    result += QLatin1Char('\n');

    // Interface Destructor
    result += commentClassDestructor;
    result += QString(QLatin1String("%1::~%1()\n")).arg(className());
    result += QLatin1String("{\n");
    result += spacing + QLatin1String("delete mPriv;\n");
    result += QLatin1String("}\n");
    result += QLatin1Char('\n');

    // Interface immutableProperties()
    result += commentMethodImmutableProperties;
    result += QString(QLatin1String("QVariantMap %1::immutableProperties() const\n")).arg(className());
    result += QLatin1String("{\n");
    result += spacing + QLatin1String("QVariantMap map;\n");

    if (m_immutablePropertiesCount) {
        creatorLine = spacing + QLatin1String("map.insert(");
        creatorSpacingStr = QString(creatorLine.size(), QLatin1Char(' '));
    }

    foreach (const CInterfaceProperty *prop, m_properties) {
        if (!prop->isImmutable()) {
            continue;
        }

        // Unchangeable properties considered as immutable, but should not be listed in this list
        if (prop->isUnchangeable()) {
//            continue;
        }

        result += creatorLine + QString(QLatin1String("%1 + QLatin1String(\".%2\"),\n")).arg(interfaceTpDefinition()).arg(prop->nameAsIs());
        result += creatorSpacingStr + QString(QLatin1String("QVariant::fromValue(%1()));\n")).arg(prop->name());
    }
    result += spacing + QLatin1String("return map;\n");
    result += QLatin1String("}\n");
    result += QLatin1Char('\n');

    // Interface properties
    foreach (const CInterfaceProperty *prop, m_properties) {
        result += QString(QLatin1String("%1 %2::%3() const\n")).arg(prop->type()).arg(className()).arg(prop->name());
        result += QLatin1String("{\n");
        result += spacing + QString(QLatin1String("return mPriv->%1;\n")).arg(prop->name());
        result += QLatin1String("}\n");
        result += QLatin1Char('\n');

        if (!prop->isImmutable()) {
            if (prop->notifier()) {
                result += QString(QLatin1String("void %1::set%2(%3)\n")).arg(className()).arg(prop->nameFirstCapital()).arg(formatArguments(prop->notifier(), /* addName*/ true));

                result += QLatin1String("{\n");
                if (prop->notifier()->isSimple()) {
                    result += spacing + QString(QLatin1String("if (mPriv->%1 == %2) {\n")).arg(prop->name()).arg(prop->notifier()->arguments.first().name());;
                    result += spacing + spacing + QLatin1String("return;\n");
                    result += spacing + QLatin1String("}\n\n");
                }
                result += spacing + QString(QLatin1String("mPriv->%1 = %2;\n")).arg(prop->name()).arg(prop->notifier()->arguments.first().name());;
                if (compatibleWithQt4) {
                    result += spacing + QString(QLatin1String("QMetaObject::invokeMethod(mPriv->adaptee, \"%1\"%2); //Can simply use emit in Qt5\n"))
                            .arg(prop->notifier()->name()).arg(formatInvokeMethodArguments(prop->notifier()));
                } else {
                    result += spacing + QString(QLatin1String("emit mPriv->adaptee->%1();\n")).arg(prop->notifier()->name());
                }
                result += QLatin1String("}\n");
                result += QLatin1Char('\n');

            } else {
                result += QString(QLatin1String("void %1::set%2(%3)\n")).arg(className()).arg(prop->nameFirstCapital()).arg(prop->formatTypeArgument(/* addName*/ true));

                result += QLatin1String("{\n");
                result += spacing + QString(QLatin1String("mPriv->%1 = %1;\n")).arg(prop->name());

                if (m_emitPropertiesChangedSignal) {
                    result += spacing + QString(QLatin1String("notifyPropertyChanged(QLatin1String(\"%1\"), QVariant::fromValue(%2));\n")).arg(prop->nameAsIs()).arg(prop->name());
                }
                result += QLatin1String("}\n");
                result += QLatin1Char('\n');
            }
        }
    }

    result += QString(QLatin1String("void %1::createAdaptor()\n")).arg(className());
    result += QLatin1String("{\n");
    result += spacing + QString(QLatin1String("(void) new %1(dbusObject()->dbusConnection(),\n")).arg(getServiceAdaptor());
    result += spacing + spacing + spacing + QLatin1String("mPriv->adaptee, dbusObject());\n");
    result += QLatin1String("}\n");
    result += QLatin1Char('\n');

    // Methods
    foreach (const CInterfaceMethod *method, m_methods) {
        result += QString(QLatin1String("void %1::set%2Callback(const %3 &cb)\n")).arg(className()).arg(method->nameFirstCapital()).arg(method->callbackType());
        result += QLatin1String("{\n");
        result += spacing + QString(QLatin1String("mPriv->%1 = cb;\n")).arg(method->callbackMember());
        result += QLatin1String("}\n");
        result += QLatin1Char('\n');

        QString checkStr = QLatin1String("{\n");
        checkStr += spacing + QString(QLatin1String("if (!mPriv->%1.isValid()) {\n")).arg(method->callbackMember());
        checkStr += spacing + spacing + QLatin1String("error->set(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String(\"Not implemented\"));\n");

        // FIXME: It's bad to check ret type for "void" and it's really bad to blindly add () for arg type otherwise. (Will not work for POD)
        if (method->callbackRetType() == QLatin1String("void")) {
            checkStr += spacing + spacing + QLatin1String("return;\n");
        } else {
            checkStr += spacing + spacing + QString(QLatin1String("return %1();\n")).arg(method->callbackRetType());
        }

        checkStr += spacing + QLatin1String("}\n");

        if (method->isSimple()) {
            result += QString(QLatin1String("%1 %2::%3(DBusError *error)\n"))
                    .arg(method->callbackRetType()).arg(className()).arg(method->name());
            result += checkStr;
            result += spacing + QString(QLatin1String("return mPriv->%1(error);\n")).arg(method->callbackMember());
        } else {
            int outputArgsCount = 0;

            for (int i = 0; i < method->arguments.count(); ++i) {
                if (method->arguments.at(i).direction() == CMethodArgument::Output) {
                    ++outputArgsCount;
                }
            }

            result += QString(QLatin1String("%1 %2::%3(%4, DBusError *error)\n"))
                    .arg(method->callbackRetType()).arg(className()).arg(method->name())
                    .arg(formatArguments(method, /* addName*/ true, /* hideOutputArguments */ (outputArgsCount <= 1)));
            result += checkStr;
            result += spacing + QString(QLatin1String("return mPriv->%1(%2, error);\n")).arg(method->callbackMember())
                    .arg(formatArguments(method, /* addName*/ true, /* hideOutputArguments */ (outputArgsCount <= 1), /* addType */ false));
        }

        result += QLatin1String("}\n");
        result += QLatin1Char('\n');
    }

    // Signals
    foreach (const CInterfaceSignal *sig, m_signals) {
        if (sig->isNotifier()) {
            continue;
        }

        result += QString(QLatin1String("void %1::%2(%3)\n")).arg(className()).arg(sig->name()).arg(formatArguments(sig, /* argName */ true));
        result += QLatin1String("{\n");
        if (compatibleWithQt4) {
            result += spacing + QString(QLatin1String("QMetaObject::invokeMethod(mPriv->adaptee, \"%1\"%2); //Can simply use emit in Qt5\n")).arg(sig->name()).arg(formatInvokeMethodArguments(sig));
        } else {
            result += spacing + QString(QLatin1String("emit mPriv->adaptee->%1();\n")).arg(sig->name());
        }
        result += QLatin1String("}\n");
        result += QLatin1Char('\n');
    }

    return result;
}

QString CInterfaceGenerator::generateMethodCallbackAndDeclaration(const CInterfaceMethod *method) const
{
    QList<uint> outputArgsIndices;

    for (int i = 0; i < method->arguments.count(); ++i) {
        if (method->arguments.at(i).direction() == CMethodArgument::Output) {
            outputArgsIndices.append(i);
        }
    }

    QString result = spacing + QLatin1String("typedef Callback");

    if (method->isSimple()) {
        result += QString(QLatin1String("1<%1, DBusError*> %2;\n")).arg(method->callbackRetType()).arg(method->callbackType());
    } else {
        result += QString(QLatin1String("%1<%2, %3, DBusError*> %4;\n"))
                .arg(method->arguments.count() + (outputArgsIndices.isEmpty() ? 1 : 0))
                .arg(method->callbackRetType()).arg(formatArguments(method, /* addName*/ false, /* hideOutputArguments */ true)).arg(method->callbackType());
    }

    result += spacing + QString(QLatin1String("void set%1Callback(const %2 &cb);\n")).arg(method->nameFirstCapital()).arg(method->callbackType());

    if (method->isSimple()) {
        result += spacing + QString(QLatin1String("%1 %2(DBusError *error);\n")).arg(method->callbackRetType()).arg(method->name());
    } else {
        result += spacing + QString(QLatin1String("%1 %2(%3, DBusError *error);\n")).arg(method->callbackRetType()).arg(method->name()).arg(formatArguments(method, /* addName*/ true, /* hideOutputArguments */ (outputArgsIndices.count() <= 1)));
    }

    return result;
}

QString CInterfaceGenerator::formatArguments(const CArgumentsFeature *argumentsClass, bool addName, bool hideOutputArguments, bool addType) const
{
    QString result;

    for (int i = 0; i < argumentsClass->arguments.count(); ++ i) {
        if (hideOutputArguments && argumentsClass->arguments.at(i).direction() == CMethodArgument::Output) {
            continue;
        }

        if (!result.isEmpty()) {
            result.append(QLatin1String(", "));
        }

        if (addType) {
            result += argumentsClass->arguments.at(i).formatArgument(addName);
        } else {
            result += argumentsClass->arguments.at(i).name();
        }
    }

    return result;
}

QString CInterfaceGenerator::formatInvokeMethodArguments(const CArgumentsFeature *argumentsClass) const
{
    QString result;

    foreach (const CMethodArgument &argument, argumentsClass->arguments) {
        result.append(QLatin1String(", "));

        result += QString(QLatin1String("Q_ARG(%1, %2)")).arg(argument.type()).arg(argument.name());
    }
    return result;
}
