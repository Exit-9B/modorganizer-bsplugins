from typing import *
import json
import sys

def push(code: TextIO, name: str, signature: Optional[str] = None,
         conflictType: str = 'Override', alignable: bool = True) -> None:
    func: str = 'getOrInsertChild' if alignable else 'insertChild'
    if signature:
        code.write(('item = item->{}('
                    'indexStack.back()++, "{}"_ts, u"{}"_s, ConflictType::{});\n'
                    ).format(func, signature, name, conflictType))
    else:
        code.write(('item = item->{}('
                    'indexStack.back()++, u"{}"_s, ConflictType::{});\n'
                    ).format(func, name, conflictType))
    code.write('indexStack.push_back(0);\n')

def pop(code: TextIO, name: str, signature: Optional[str] = None) -> None:
    comment: str
    if signature:
        comment = signature + ' - ' + name
    else:
        comment = name
    code.write('item = item->parent();\n')
    code.write('indexStack.pop_back(); // {}\n\n'.format(comment))

class FormatValue:
    def divide(code: TextIO, format: dict[str, Any]) -> None:
        value: int = int(format['value'])
        code.write('val = val.toInt() / {}.0f;\n'.format(value))

    def enum(code: TextIO, format: dict[str, Any]) -> None:
        options: dict[str, str] = format['options']
        code.write('switch (val.toUInt()) {\n')

        val: str
        label: str
        for val, label in options.items():
            if val.isdigit():
                code.write('case {}U:\n'.format(val))
            else:
                code.write('case "{}"_ts.value:\n'.format(val))
            code.write('val = u"{}"_s;\n'.format(label))
            code.write('break;\n')
        code.write('}\n')

    def flags(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: flags\n')

    def AtxtPositionFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: AtxtPositionFormat\n')

    def ClmtMoonsPhaseLengthFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: ClmtMoonsPhaseLengthFormat\n')

    def ClmtTimeFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: ClmtTimeFormat\n')

    def CloudSpeedFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: CloudSpeedFormat\n')

    def CTDAFunctionFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: CTDAFunctionFormat\n')

    def CtdaTypeFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: CtdaTypeFormat\n')

    def Edge0Format(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: Edge0Format\n')

    def Edge1Format(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: Edge1Format\n')

    def Edge2Format(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: Edge2Format\n')

    def HideFFFF_Format(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: HideFFFF_Format\n')

    def NextObjectIDFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: NextObjectIDFormat\n')

    def QuestAliasFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: QuestAliasFormat\n')

    def QuestExternalAliasFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: QuestExternalAliasFormat\n')

    def REFRNavmeshTriangleFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: REFRNavmeshTriangleFormat\n')

    def ScriptObjectAliasFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: ScriptObjectAliasFormat\n')

    def TintLayerFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: TintLayerFormat\n')

    def Vertex0Format(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: Vertex0Format\n')

    def Vertex1Format(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: Vertex1Format\n')

    def Vertex2Format(code: TextIO, format: dict[str, Any]) -> None:
        code.write('// TODO: Vertex2Format\n')

def format_val(code: TextIO, format: dict[str, Any], defs: dict[str, Any]) -> None:
    if 'id' in format:
        format = defs[format['id']]

    type: str = format['type']
    getattr(FormatValue, type)(code, format)

class UnionDecider:
    def GMSTUnionDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('enum {Name, Int, Float, Bool};\n'
                   'switch (root->childData("EDID"_ts, fileIndex)'
                   '.toString()[0].unicode()) {\n'
                   "case u'b': decider = Bool; break;\n"
                   "case u'f': decider = Float; break;\n"
                   "case u'i': decider = Int; break;\n"
                   "case u'u': decider = Int; break;\n"
                   "case u's': decider = Name; break;\n"
                   '}\n')
        return True
    def CTDACompValueDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: CTDACompValueDecider\n')
        return False
    def CTDAParam1Decider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: CTDAParam1Decider\n')
        return False
    def CTDAParam2Decider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: CTDAParam1Decider\n')
        return False
    def CTDAReferenceDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: CTDAReferenceDecider\n')
        return False
    def ScriptPropertyDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('enum {Unused, ObjectUnion, String, Int32, Float, Bool, '
                   'ArrayofObject, ArrayofString, ArrayofInt32, ArrayofFloat, '
                   'ArrayofBool};\n'
                   'const QString propertyType = '
                   'item->parent()->childData(u"Type"_s, fileIndex).toString();\n'
                   'if (propertyType == u"None"_s) {\n'
                   'decider = Unused;\n'
                   '} else if (propertyType == u"Object"_s) {\n'
                   'decider = ObjectUnion;\n'
                   '} else if (propertyType == u"String"_s) {\n'
                   'decider = String;\n'
                   '} else if (propertyType == u"Int32"_s) {\n'
                   'decider = Int32;\n'
                   '} else if (propertyType == u"Float"_s) {\n'
                   'decider = Float;\n'
                   '} else if (propertyType == u"Bool"_s) {\n'
                   'decider = Bool;\n'
                   '} else if (propertyType == u"Array of Object"_s) {\n'
                   'decider = ArrayofObject;\n'
                   '} else if (propertyType == u"Array of String"_s) {\n'
                   'decider = ArrayofString;\n'
                   '} else if (propertyType == u"Array of Int32"_s) {\n'
                   'decider = ArrayofInt32;\n'
                   '} else if (propertyType == u"Array of Float"_s) {\n'
                   'decider = ArrayofFloat;\n'
                   '} else if (propertyType == u"Array of Bool"_s) {\n'
                   'decider = ArrayofBool;\n'
                   '}\n')
        return True
    def ScriptObjFormatDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('enum {Objectv2, Objectv1};\n'
                   'decider = ObjectFormat == 1 ? Objectv1 : Objectv2;')
        return True
    def TypeDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: TypeDecider\n')
        return False
    def BOOKTeachesDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: BOOKTeachesDecider\n')
        return False
    def COEDOwnerDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: COEDOwnerDecider\n')
        return False
    def MGEFAssocItemDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: MGEFAssocItemDecider\n')
        return False
    def NAVIIslandDataDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: NAVIIslandDataDecider\n')
        return False
    def NAVIParentDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: NAVIParentDecider\n')
        return False
    def NVNMParentDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: NVNMParentDecider\n')
        return False
    def NPCLevelDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: NPCLevelDecider\n')
        return False
    def PubPackCNAMDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: PubPackCNAMDecider\n')
        return False
    def PerkDATADecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: PerkDataDecider\n')
        return False
    def EPFDDecider(code: TextIO, element: dict[str, Any]) -> bool:
        code.write('// TODO: EPFDDecider\n')
        return False

class DefineType:
    def int0(code: TextIO, element: dict[str, Any]) -> None:
        code.write('QVariant val = 0;\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    def int8(code: TextIO, element: dict[str, Any]) -> None:
        code.write('QVariant val = '
                   'TESFile::readType<std::int8_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    def int16(code: TextIO, element: dict[str, Any]) -> None:
        code.write('QVariant val = TESFile::readType<std::int16_t>(*stream);\n')
        if 'name' in element and element['name'] == 'Object Format':
            code.write('ObjectFormat = val.toInt();\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    def int32(code: TextIO, element: dict[str, Any]) -> None:
        code.write('QVariant val = TESFile::readType<std::int32_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    def uint8(code: TextIO, element: dict[str, Any]) -> None:
        code.write('QVariant val = TESFile::readType<std::uint8_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    def uint16(code: TextIO, element: dict[str, Any]) -> None:
        code.write('QVariant val = TESFile::readType<std::uint16_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    def uint32(code: TextIO, element: dict[str, Any]) -> None:
        code.write('QVariant val = TESFile::readType<std::uint32_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    def float(code: TextIO, element: dict[str, Any]) -> None:
        code.write('QVariant val = TESFile::readType<float>(*stream);\n')
        code.write('item->setDisplayData(fileIndex, val);\n')

    def string(code: TextIO, element: dict[str, Any]) -> None:
        localized: bool = element.get('localized', False)
        if localized:
            code.write('item->setDisplayData('
                       'fileIndex, readLstring(localized, *stream));\n')
        elif 'prefix' in element:
            prefix: int = element['prefix']
            lengthType: str
            if prefix == 1:
                lengthType = 'std::uint8_t'
            elif prefix == 2:
                lengthType = 'std::uint16_t'
            elif prefix == 4:
                lengthType = 'std::uint32_t'
            code.write((
                'const auto length = TESFile::readType<{}>(*stream);\n'
                'std::string str;\n'
                'str.resize(length);\n'
                'stream->read(str.data(), length);\n'
                'item->setDisplayData(fileIndex, QString::fromStdString(str));\n'
                ).format(lengthType))
        else:
            code.write(
                'std::string str;\n'
                "std::getline(*stream, str, '\\0');\n"
                'item->setDisplayData(fileIndex, QString::fromStdString(str));\n')

    def formId(code: TextIO, element: dict[str, Any]) -> None:
        code.write('item->setDisplayData('
                   'fileIndex, readFormId(masters, plugin, *stream));\n')

    def bytes(code: TextIO, element: dict[str, Any]) -> None:
        size: int = element.get('size', 256)
        code.write('item->setDisplayData(fileIndex, readBytes(*stream, {}));\n'.format(
            size))

    def array(code: TextIO, element: dict[str, Any]) -> None:
        name: str = element['name']
        code.write('if (stream->peek() != std::char_traits<char>::eof()) {\n')
        if 'count' in element:
            count: int = element['count']
            code.write(('for ([[maybe_unused]] int i_{} : '
                        'std::ranges::iota_view(0, {})) {{\n'
                        ).format(name.replace(' ', ''), count))
        elif 'counter' in element:
            nameId: str = name.replace(' ', '')
            counter: dict[str, Any] = element['counter']
            counterType: str = counter['type']
            if counterType == 'elementCounter':
                path: str = counter['path']
                code.write(('const int count_{} = '
                            'root->childData(u"{}"_s, fileIndex).toInt();\n'
                            ).format(nameId, path))
            elif counterType == 'ScriptFragmentsInfoCounter':
                code.write(('const int count_{} = std::popcount('
                            'item->parent()->childData('
                            'u"Flags"_s, fileIndex).toUInt() & 0x3U);\n'
                            ).format(nameId))
            elif counterType == 'ScriptFragmentsPackCounter':
                code.write(('const int count_{} = std::popcount('
                            'item->parent()->childData('
                            'u"Flags"_s, fileIndex).toUInt() & 0x7U);\n'
                            ).format(nameId))
            elif counterType == 'ScriptFragmentsQuestCounter':
                code.write(('const int count_{} = '
                            'item->parent()->childData('
                            'u"FragmentCount"_s, fileIndex)'
                            '.toInt();\n').format(nameId))
            elif counterType == 'ScriptFragmentsSceneCounter':
                code.write(('const int count_{} = std::popcount('
                            'item->parent()->childData('
                            'u"Flags"_s, fileIndex).toUInt() & 0x3U);\n'
                            ).format(nameId))
            else:
                code.write('#pragma message("warning: unknown counter type {}")'.format(
                    counterType))
            code.write(('for ([[maybe_unused]] int i_{0} : '
                        'std::ranges::iota_view(0, count_{0})) {{\n'
                        ).format(nameId))
        elif 'prefix' in element:
            nameId: str = name.replace(' ', '').replace('?', '')
            prefix: int = element['prefix']
            if prefix == 1:
                code.write(('const int count_{} = '
                           'TESFile::readType<std::uint8_t>(*stream);\n'
                           ).format(nameId))
            elif prefix == 2:
                code.write(('const int count_{} = '
                            'TESFile::readType<std::uint16_t>(*stream);\n'
                            ).format(nameId))
            elif prefix == 4:
                code.write(('const int count_{} = '
                            'TESFile::readType<std::uint32_t>(*stream);\n'
                            ).format(nameId))
            code.write(('for ([[maybe_unused]] int i_{0} : '
                        'std::ranges::iota_view(0, count_{0})) {{\n'
                        ).format(nameId))
        else:
            code.write('while (!stream->eof()) {\n')
        arrayElement: dict[str, Any] = element['element']
        if 'id' in arrayElement:
            arrayElement = defs[arrayElement['id']]
        elementName: str = arrayElement['name']
        push(code, elementName)
        define_type(code, arrayElement, defs)
        pop(code, elementName)
        code.write('}\n')
        code.write('}\n')

    def struct(code: TextIO, element: dict[str, Any]) -> None:
        name: str = element['name']

        code.write('if (stream->peek() != std::char_traits<char>::eof()) {\n')
        structElements: list[Any] = element['elements']
        structElement: dict[str, Any]
        for structElement in structElements:
            if 'id' in structElement:
                structElement = {**structElement, **defs[structElement['id']]}
                del structElement['id']
            conflictType = structElement.get('conflictType', 'Override')
            elementName: str = structElement['name']
            push(code, elementName, conflictType=conflictType)
            code.write('{\n')
            define_type(code, structElement, defs)
            code.write('}\n')
            pop(code, elementName)
        code.write('}\n')

    def union(code: TextIO, element: dict[str, Any]) -> None:
        decider: str = element['decider']
        code.write('[[maybe_unused]] int decider = -1;\n')
        if not getattr(UnionDecider, decider)(code, element):
            return

        unionElements: list[Any] = element['elements']
        unionElement: dict[str, Any]
        code.write('switch (decider) {\n')
        for unionElement in unionElements:
            if 'id' in unionElement:
                unionElement = defs[unionElement['id']]
            name: str = unionElement['name'].replace(' ', '')
            code.write('case {}: {{\n'.format(name))
            define_type(code, unionElement, defs)
            code.write('} break;\n')
        code.write('}\n')

    def empty(code: TextIO, element: dict[str, Any]) -> None:
        code.write('// TODO: empty\n')

    def memberStruct(code: TextIO, element: dict[str, Any]) -> None:
        #define_member(code, element, defs)
        members: list[Any] = element['members']
        structMember: dict[str, Any]
        for structMember in members:
            if 'id' in structMember:
                structMember = {**structMember, **defs[structMember['id']]}
                del structMember['id']
            structType: str = structMember['type']
            if structType.startswith('member'):
                define_member(code, structMember, defs)
            else:
                structSig: str = structMember['signature'].encode('unicode_escape').decode()
                structName = structMember.get('name', '')
                push(code, structName, signature=structSig)
                code.write('if (signature == "{}"_ts) {{\n'.format(structSig))
                define_type(code, structMember, defs)
                code.write('co_await std::suspend_always();\n}\n')
                pop(code, structName, signature=structSig)

    def memberUnion(code: TextIO, element: dict[str, Any]) -> None:
        define_member(code, element, defs)

def define_type(code: TextIO, element: dict[str, Any], defs: dict[str, Any]) -> None:
    if 'id' in element:
        element = defs[element['id']]

    type: str = element['type']
    getattr(DefineType, type)(code, element)

class MemberCondition:
    def memberArray(member: dict[str, Any], defs: dict[str, Any]) -> str:
        return member_condition(member['member'], defs)

    def memberStruct(member: dict[str, Any], defs: dict[str, Any]) -> str:
        return member_condition(member['members'][0], defs)

    def memberUnion(member: dict[str, Any], defs: dict[str, Any]) -> str:
        return ' || '.join(
            (member_condition(unionMember, defs)
             for unionMember in member['members']))

    def default(member: dict[str, Any], defs: dict[str, Any]) -> str:
        signature: str = member['signature'].encode('unicode_escape').decode()
        return 'signature == "{}"_ts'.format(signature)

def member_condition(member: dict[str, Any], defs: dict[str, Any]) -> str:
    if 'id' in member:
        member = defs[member['id']]
    memberType = member['type']
    return getattr(MemberCondition, memberType, MemberCondition.default)(member, defs)

class DefineMember:
    def memberArray(code: TextIO, member: dict[str, Any], defs: dict[str, Any]) -> None:
        name: str = member.get('name', 'Unknown')
        conflictType: str = member.get('conflictType', 'Override')
        alignable: bool = True
        if 'defFlags' in member:
            defFlags: list[str] = member['defFlags']
            if 'notAlignable' in defFlags:
                alignable = False

        arrayMember: dict[str, Any] = member['member']
        if 'id' in arrayMember:
            arrayMember = defs[arrayMember['id']]
        arrayName: str = ''
        if 'name' in arrayMember:
            arrayName = arrayMember['name']
        push(code, name, conflictType=conflictType)
        code.write('while ({}) {{\n'.format(member_condition(arrayMember, defs)))

        arrayType: str = arrayMember['type']
        if arrayType.startswith('member'):
            define_member(code, arrayMember, defs)
        else:
            arraySig: str = arrayMember.get('signature')
            if arraySig:
                arraySig = arraySig.encode('unicode_escape').decode()
            push(code, arrayName, signature=arraySig, conflictType=conflictType,
                 alignable=alignable)

            define_type(code, arrayMember, defs)
            pop(code, arrayName, signature=arraySig)

            if 'signature' in arrayMember:
                code.write('co_await std::suspend_always();\n')
        code.write('}\n')
        pop(code, name)

    def memberStruct(code: TextIO, member: dict[str, Any], defs: dict[str, Any]
                     ) -> None:
        name: str = member.get('name', 'Unknown')
        conflictType: str = member.get('conflictType', 'Override')

        push(code, name, conflictType=conflictType)
        members: list[Any] = member['members']

        structMember: dict[str, Any]
        for structMember in members:
            if 'id' in structMember:
                structMember = {**structMember, **defs[structMember['id']]}
                del structMember['id']
            structType: str = structMember['type']
            if structType.startswith('member'):
                define_member(code, structMember, defs)
            else:
                structSig: str = structMember['signature'].encode('unicode_escape').decode()
                structName: str = ''
                if 'name' in structMember:
                    structName: str = structMember['name']
                push(code, structName, signature=structSig)
                code.write('if (signature == "{}"_ts) {{\n'.format(structSig))
                define_type(code, structMember, defs)
                code.write('co_await std::suspend_always();\n}\n')
                pop(code, structName, signature=structSig)
        pop(code, name)

    def memberUnion(code: TextIO, member: dict[str, Any], defs: dict[str, Any]) -> None:
        name: str = member.get('name', 'Unknown')
        conflictType: str = member.get('conflictType', 'Override')

        push(code, name)
        members: list[Any] = member['members']

        unionMember: dict[str, Any]
        for unionMember in members:
            if 'id' in unionMember:
                unionMember = defs[unionMember['id']]
            unionType: str = unionMember['type']
            code.write('if ({}) {{\n'.format(member_condition(unionMember, defs)))
            if unionType.startswith('member'):
                define_member(code, unionMember, defs)
            else:
                unionSig: str = unionMember['signature'].encode('unicode_escape').decode()
                unionName: str = ''
                if 'name' in unionMember:
                    unionName: str = unionMember['name']
                code.write('if (signature == "{}"_ts) {{\n'.format(unionSig))
                push(code, unionName, signature=unionSig)
                define_type(code, unionMember, defs)
                pop(code, unionName, signature=unionSig)
                code.write('co_await std::suspend_always();\n}\n')
            code.write('}\n')
        pop(code, name)

    def default(code: TextIO, member: dict[str, Any], defs: dict[str, Any]) -> None:
        name: str = member.get('name', 'Unknown')
        conflictType: str = member.get('conflictType', 'Override')

        signature: str = member['signature'].encode('unicode_escape').decode()
        if signature == 'VMAD':
            code.write('int ObjectFormat;\n')
        push(code, name, signature=signature)
        code.write('if (signature == "{}"_ts) {{\n'.format(signature))
        define_type(code, member, defs)
        code.write('co_await std::suspend_always();\n}\n')
        pop(code, name, signature=signature)

def define_member(code: TextIO, member: dict[str, Any], defs: dict[str, Any]) -> None:
    type: str = member['type']
    getattr(DefineMember, type, DefineMember.default)(code, member, defs)

def define_record(code: TextIO, definition: dict[str, Any], defs: dict[str, Any]) -> str:
    signature: str = definition['signature'].encode('unicode_escape').decode()
    if 'id' in definition:
        definition = {**definition, **defs[definition['id']]}

    code.write((
        'template <>\n'
        'ParseTask FormParser<"{}">::parseForm('
        '    DataItem* root, int fileIndex, [[maybe_unused]] bool localized,\n'
        '    [[maybe_unused]] std::span<const std::string> masters,\n'
        '    [[maybe_unused]] const std::string& plugin, const TESFile::Type& signature,\n'
        '    std::istream* const& stream) const\n'
        '{{\n'
        'using ConflictType = DataItem::ConflictType;'
        'DataItem* item = root;\n'
        'std::vector<int> indexStack{{0}};\n'
        '\n').format(signature))
    members: list[Any] = definition['members']
    member: dict[str, Any]
    for member in members:
        if 'id' in member:
            member = {**member, **defs[member['id']]}
            del member['id']
        define_member(code, member, defs)

    code.write('for (;;) {\n'
               'parseUnknown('
               'root, indexStack.front(), fileIndex, signature, *stream);\n'
               'co_await std::suspend_always();\n'
               '}\n')
    code.write('}\n\n')

if __name__ == '__main__':
    dataPath: str = sys.argv[1]
    outPath: str = sys.argv[2]

    dataFile: TextIO
    with open(dataPath, 'r') as dataFile:
        data: dict[str, Any] = json.load(dataFile)
        defs: dict[str, Any] = data['defs']

        with open(outPath, 'w') as outFile:
            outFile.write('namespace TESData\n{\n\n')

            id: str
            definition: dict[str, Any]
            for id, definition in defs.items():
                type: str = definition['type']
                if type == 'record' and 'signature' in definition:
                    define_record(outFile, definition, defs)

            outFile.write('\n}\n')
