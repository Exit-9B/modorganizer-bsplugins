from typing import *
import json
import sys

def format_val(format: dict[str, Any], defs: dict[str, Any]) -> str:
    if 'id' in format:
        format = defs[format['id']]

    type: str = format['type']
    if type == 'divide':
        value: int = int(format['value'])
        return 'val = val.toInt() / {}.0f;\n'.format(value)
    elif type == 'enum':
        options: dict[str, str] = format['options']
        code: str = 'switch (val.toUInt()) {\n'

        val: str
        label: str
        for val, label in options.items():
            if val.isdigit():
                code += 'case {}U:\n'.format(val)
            else:
                code += 'case "{}"_ts.value:\n'.format(val)
            code += 'val = u"{}"_s;\n'.format(label)
            code += 'break;\n'
        code += '}\n'
        return code
    elif type == 'flags':
        return '// TODO: flags\n'
    elif type == 'AtxtPositionFormat':
        return '// TODO\n'
    elif type == 'ClmtMoonsPhaseLengthFormat':
        return '// TODO\n'
    elif type == 'ClmtTimeFormat':
        return '// TODO\n'
    elif type == 'CloudSpeedFormat':
        return '// TODO\n'
    elif type == 'CTDAFunctionFormat':
        return '// TODO\n'
    elif type == 'CtdaTypeFormat':
        return '// TODO\n'
    elif type == 'Edge0Format':
        return '// TODO\n'
    elif type == 'Edge1Format':
        return '// TODO\n'
    elif type == 'Edge2Format':
        return '// TODO\n'
    elif type == 'HideFFFF_Format':
        return '// TODO\n'
    elif type == 'NextObjectIDFormat':
        return '// TODO\n'
    elif type == 'QuestAliasFormat':
        return '// TODO\n'
    elif type == 'QuestExternalAliasFormat':
        return '// TODO\n'
    elif type == 'REFRNavmeshTriangleFormat':
        return '// TODO\n'
    elif type == 'ScriptObjectAliasFormat':
        return '// TODO\n'
    elif type == 'TintLayerFormat':
        return '// TODO: TintLayerFormat\n'
    elif type == 'Vertex0Format':
        return '// TODO\n'
    elif type == 'Vertex1Format':
        return '// TODO\n'
    elif type == 'Vertex2Format':
        return '// TODO\n'
    else:
        return '#pragma message("warning: unhandled format type {}")\n'.format(type)

def define_type(element: dict[str, Any], defs: dict[str, Any]) -> str:
    if 'id' in element:
        element = defs[element['id']]

    type: str = element['type']
    if type == 'int0':
        code: str = 'QVariant val = 0;\n'
        if 'format' in element:
            format: dict[str, Any] = element['format']
            code += format_val(format, defs)
        code += 'item->setDisplayData(fileIndex, val);\n'
        return code

    if type == 'int8':
        code: str = ('QVariant val = '
                     'TESFile::readType<std::int8_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            code += format_val(format, defs)
        code += 'item->setDisplayData(fileIndex, val);\n'
        return code

    elif type == 'int16':
        code: str = ('QVariant val = '
                     'TESFile::readType<std::int16_t>(*stream);\n')
        if 'name' in element and element['name'] == 'Object Format':
            code += 'ObjectFormat = val.toInt();\n'
        if 'format' in element:
            format: dict[str, Any] = element['format']
            code += format_val(format, defs)
        code += 'item->setDisplayData(fileIndex, val);\n'
        return code

    elif type == 'int32':
        code: str = ('QVariant val = '
                     'TESFile::readType<std::int32_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            code += format_val(format, defs)
        code += 'item->setDisplayData(fileIndex, val);\n'
        return code

    elif type == 'uint8':
        code: str = ('QVariant val = '
                     'TESFile::readType<std::uint8_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            code += format_val(format, defs)
        code += 'item->setDisplayData(fileIndex, val);\n'
        return code

    elif type == 'uint16':
        code: str = ('QVariant val = '
                     'TESFile::readType<std::uint16_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            code += format_val(format, defs)
        code += 'item->setDisplayData(fileIndex, val);\n'
        return code

    elif type == 'uint32':
        code: str = ('QVariant val = '
                     'TESFile::readType<std::uint32_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            code += format_val(format, defs)
        code += 'item->setDisplayData(fileIndex, val);\n'
        return code

    elif type == 'float':
        code: str = 'QVariant val = TESFile::readType<float>(*stream);\n'
        code += 'item->setDisplayData(fileIndex, val);\n'
        return code

    elif type == 'string':
        localized: bool = 'localized' in element and element['localized']
        if localized:
            return 'item->setDisplayData(fileIndex, readLstring(localized, *stream));\n'
        elif 'prefix' in element:
            prefix: int = element['prefix']
            lengthType: str
            if prefix == 1:
                lengthType = 'std::uint8_t'
            elif prefix == 2:
                lengthType = 'std::uint16_t'
            elif prefix == 4:
                lengthType = 'std::uint32_t'
            code: str = (
                'const auto length = TESFile::readType<{}>(*stream);\n'
                'std::string str;\n'
                'str.resize(length);\n'
                'stream->read(str.data(), length);\n'
                'item->setDisplayData(fileIndex, QString::fromStdString(str));\n'
                ).format(lengthType)
            return code
        else:
            return (
                'std::string str;\n'
                "std::getline(*stream, str, '\\0');\n"
                'item->setDisplayData(fileIndex, QString::fromStdString(str));\n')
    elif type == 'formId':
        return 'item->setDisplayData(fileIndex, readFormId(masters, plugin, *stream));\n'
    elif type == 'bytes':
        size: int = element['size'] if 'size' in element else 256
        return 'item->setDisplayData(fileIndex, readBytes(*stream, {}));\n'.format(size)
    elif type == 'array':
        name: str = element['name']
        code: str = 'if (stream->peek() != std::char_traits<char>::eof()) {\n'
        if 'count' in element:
            count: int = element['count']
            code += ('for ([[maybe_unused]] int i_{} : '
                     'std::ranges::iota_view(0, {})) {{\n'
                     ).format(name.replace(' ', ''), count)
        elif 'counter' in element:
            nameId: str = name.replace(' ', '')
            counter: dict[str, Any] = element['counter']
            counterType: str = counter['type']
            if counterType == 'elementCounter':
                path: str = counter['path']
                code += ('const int count_{} = '
                         'root->childData(u"{}"_s, fileIndex).toInt();\n'
                         ).format(nameId, path)
            elif counterType == 'ScriptFragmentsInfoCounter':
                code += ('const int count_{} = std::popcount('
                         'item->parent()->childData('
                         'u"Flags"_s, fileIndex).toUInt() & 0x3U);\n'
                         ).format(nameId)
            elif counterType == 'ScriptFragmentsPackCounter':
                code += ('const int count_{} = std::popcount('
                         'item->parent()->childData('
                         'u"Flags"_s, fileIndex).toUInt() & 0x7U);\n').format(nameId)
            elif counterType == 'ScriptFragmentsQuestCounter':
                code += ('const int count_{} = '
                         'item->parent()->childData('
                         'u"FragmentCount"_s, fileIndex)'
                         '.toInt();\n').format(nameId)
            elif counterType == 'ScriptFragmentsSceneCounter':
                code += ('const int count_{} = std::popcount('
                         'item->parent()->childData('
                         'u"Flags"_s, fileIndex).toUInt() & 0x3U);\n').format(nameId)
            else:
                return '#pragma message("warning: unknown counter type {}")'.format(
                    counterType)
            code += ('for ([[maybe_unused]] int i_{0} : '
                     'std::ranges::iota_view(0, count_{0})) {{\n'
                     ).format(nameId)
        elif 'prefix' in element:
            nameId: str = name.replace(' ', '').replace('?', '')
            prefix: int = element['prefix']
            if prefix == 1:
                code += ('const int count_{} = '
                         'TESFile::readType<std::uint8_t>(*stream);\n').format(nameId)
            elif prefix == 2:
                code += ('const int count_{} = '
                         'TESFile::readType<std::uint16_t>(*stream);\n').format(nameId)
            elif prefix == 4:
                code += ('const int count_{} = '
                         'TESFile::readType<std::uint32_t>(*stream);\n').format(nameId)
            code += ('for ([[maybe_unused]] int i_{0} : '
                     'std::ranges::iota_view(0, count_{0})) {{\n'
                     ).format(nameId)
        else:
            code += 'while (!stream->eof()) {\n'
        arrayElement: dict[str, Any] = element['element']
        if 'id' in arrayElement:
            arrayElement = defs[arrayElement['id']]
        code += ('item = item->getOrInsertChild('
                 'indexStack.back()++, u"{}"_s);\n').format(arrayElement['name'])
        code += 'indexStack.push_back(0);\n'
        code += define_type(arrayElement, defs)
        code += 'item = item->parent();\n'
        code += 'indexStack.pop_back();\n\n'
        code += '}\n'
        code += '}\n'
        return code
    elif type == 'struct':
        name: str = element['name']

        code: str = 'if (stream->peek() != std::char_traits<char>::eof()) {\n'
        structElements: list[Any] = element['elements']
        structElement: dict[str, Any]
        for structElement in structElements:
            if 'id' in structElement:
                structElement = {**structElement, **defs[structElement['id']]}
                del structElement['id']
            conflictType: str = (structElement['conflictType']
                                 if 'conflictType' in structElement
                                 else 'Override')
            elementName: str = structElement['name']
            code += ('item = item->getOrInsertChild('
                     'indexStack.back()++, u"{}"_s, ConflictType::{});\n'
                     ).format(elementName, conflictType)
            code += 'indexStack.push_back(0);\n'
            code += '{\n'
            code += define_type(structElement, defs)
            code += '}\n'
            code += 'item = item->parent();\n'
            code += 'indexStack.pop_back();\n\n'
        code += '}\n'
        return code
    elif type == 'union':
        decider: str = element['decider']
        code: str = 'int decider = -1;\n'
        if decider == 'GMSTUnionDecider':
            code += ('enum {Name, Int, Float, Bool};\n'
                     'switch (root->childData("EDID"_ts, fileIndex)'
                     '.toString()[0].unicode()) {\n'
                     "case u'b': decider = Bool; break;\n"
                     "case u'f': decider = Float; break;\n"
                     "case u'i': decider = Int; break;\n"
                     "case u'u': decider = Int; break;\n"
                     "case u's': decider = Name; break;\n"
                     '}\n')
        elif decider == 'CTDACompValueDecider':
            return '// TODO: CTDACompValueDecider\n'
        elif decider == 'CTDAParam1Decider':
            return '// TODO: CTDAParam1Decider\n'
        elif decider == 'CTDAParam2Decider':
            return '// TODO: CTDAParam1Decider\n'
        elif decider == 'CTDAReferenceDecider':
            return '// TODO: CTDAReferenceDecider\n'
        elif decider == 'ScriptPropertyDecider':
            code += ('enum {Unused, ObjectUnion, String, Int32, Float, Bool, '
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
        elif decider == 'ScriptObjFormatDecider':
            code += ('enum {Objectv2, Objectv1};\n'
                     'decider = ObjectFormat == 1 ? Objectv1 : Objectv2;')
        elif decider == 'TypeDecider':
            return '// TODO: TypeDecider\n'
        elif decider == 'BOOKTeachesDecider':
            return '// TODO: BOOKTeachesDecider\n'
        elif decider == 'COEDOwnerDecider':
            return '// TODO: COEDOwnerDecider\n'
        elif decider == 'MGEFAssocItemDecider':
            return '// TODO: MGEFAssocItemDecider\n'
        elif decider == 'NAVIIslandDataDecider':
            return '// TODO: NAVIIslandDataDecider\n'
        elif decider == 'NAVIParentDecider':
            return '// TODO: NAVIParentDecider\n'
        elif decider == 'NVNMParentDecider':
            return '// TODO: NVNMParentDecider\n'
        elif decider == 'NPCLevelDecider':
            return '// TODO: NPCLevelDecider\n'
        elif decider == 'PubPackCNAMDecider':
            return '// TODO: PubPackCNAMDecider\n'
        elif decider == 'PerkDATADecider':
            return '// TODO: PerkDataDecider\n'
        elif decider == 'EPFDDecider':
            return '// TODO: EPFDDecider\n'
        else:
            return '// TODO: union decider {}\n'.format(decider)

        unionElements: list[Any] = element['elements']
        unionElement: dict[str, Any]
        code += 'switch (decider) {\n'
        for unionElement in unionElements:
            if 'id' in unionElement:
                unionElement = defs[unionElement['id']]
            name: str = unionElement['name'].replace(' ', '')
            code += 'case {}: {{\n'.format(name)
            code += define_type(unionElement, defs)
            code += '} break;\n'
        code += '}\n'
        return code
    elif type == 'empty':
        return '// TODO: empty\n'
    elif type == 'memberStruct':
        #return define_member(element, defs)
        code: str = ''
        members: list[Any] = element['members']
        structMember: dict[str, Any]
        for structMember in members:
            if 'id' in structMember:
                structMember = {**structMember, **defs[structMember['id']]}
                del structMember['id']
            structType: str = structMember['type']
            if structType.startswith('member'):
                code += define_member(structMember, defs)
            else:
                structSig: str = structMember['signature'].encode('unicode_escape').decode()
                structName: str = ''
                if 'name' in structMember:
                    structName: str = structMember['name']
                code += ('item = item->getOrInsertChild('
                         'indexStack.back()++, "{}"_ts, u"{}"_s);\n').format(
                             structSig, structName)
                code += 'indexStack.push_back(0);\n'
                code += 'if (signature == "{}"_ts) {{\n'.format(structSig)
                code += define_type(structMember, defs)
                code += 'co_await std::suspend_always();\n}\n'
                code += 'item = item->parent();\n'
                code += 'indexStack.pop_back();\n\n'
        return code
    elif type == 'memberUnion':
        return define_member(element, defs)
    else:
        return '#pragma message("warning: unhandled type {}")\n'.format(type)

def member_condition(member: dict[str, Any], defs: dict[str, Any]) -> str:
    if 'id' in member:
        member = defs[member['id']]
    memberType = member['type']
    if memberType == 'memberArray':
        return member_condition(member['member'], defs)
    if memberType == 'memberStruct':
        return member_condition(member['members'][0], defs)
    elif memberType == 'memberUnion':
        return ' || '.join(
            (member_condition(unionMember, defs)
             for unionMember in member['members']))
    else:
        signature: str = member['signature'].encode('unicode_escape').decode()
        return 'signature == "{}"_ts'.format(signature)

def define_member(member: dict[str, Any], defs: dict[str, Any]) -> str:
    name: str = member['name'] if 'name' in member else 'Unknown'
    conflictType: str = (member['conflictType'] if 'conflictType' in member
                         else 'Override')

    alignable: bool = True
    if 'defFlags' in member:
        defFlags: list[str] = member['defFlags']
        if 'notAlignable' in defFlags:
            alignable = False

    type: str = member['type']
    if type == 'memberArray':
        arrayMember: dict[str, Any] = member['member']
        if 'id' in arrayMember:
            arrayMember = defs[arrayMember['id']]
        arrayName: str = ''
        if 'name' in arrayMember:
            arrayName = arrayMember['name']
        code: str = ('item = item->getOrInsertChild('
                     'indexStack.back()++, u"{}"_s, ConflictType::{});\n'
                     ).format(name, conflictType)
        code += 'indexStack.push_back(0);\n'
        code += 'while ({}) {{\n'.format(member_condition(arrayMember, defs))

        arrayType: str = arrayMember['type']
        if arrayType.startswith('member'):
            code += define_member(arrayMember, defs)
        else:
            func: str = 'getOrInsertChild' if alignable else 'insertChild'
            if 'signature' in arrayMember:
                arraySig: str = arrayMember['signature'].encode('unicode_escape').decode()
                code += ('item = item->{}('
                         'indexStack.back()++, "{}"_ts, u"{}"_s, ConflictType::{});\n'
                         ).format(func, arraySig, arrayName, conflictType)
            else:
                code += ('item = item->{}('
                         'indexStack.back()++, u"{}"_s);\n').format(func, arrayName)
            code += 'indexStack.push_back(0);\n'

            code += define_type(arrayMember, defs)
            code += 'item = item->parent();\n'
            code += 'indexStack.pop_back();\n\n'

            if 'signature' in arrayMember:
                code += 'co_await std::suspend_always();\n'
        code += '}\n'
        code += 'item = item->parent();\n'
        code += 'indexStack.pop_back();\n\n'
        return code
    elif type == 'memberStruct':
        code: str = ('item = item->getOrInsertChild('
                     'indexStack.back()++, u"{}"_s, ConflictType::{});\n'
                     ).format(name, conflictType)
        code += 'indexStack.push_back(0);\n'
        members: list[Any] = member['members']

        structMember: dict[str, Any]
        for structMember in members:
            if 'id' in structMember:
                structMember = {**structMember, **defs[structMember['id']]}
                del structMember['id']
            structType: str = structMember['type']
            if structType.startswith('member'):
                code += define_member(structMember, defs)
            else:
                structSig: str = structMember['signature'].encode('unicode_escape').decode()
                structName: str = ''
                if 'name' in structMember:
                    structName: str = structMember['name']
                code += ('item = item->getOrInsertChild('
                         'indexStack.back()++, "{}"_ts, u"{}"_s);\n').format(
                             structSig, structName)
                code += 'indexStack.push_back(0);\n'
                code += 'if (signature == "{}"_ts) {{\n'.format(structSig)
                code += define_type(structMember, defs)
                code += 'co_await std::suspend_always();\n}\n'
                code += 'item = item->parent();\n'
                code += 'indexStack.pop_back();\n\n'
        code += 'item = item->parent();\n'
        code += 'indexStack.pop_back();\n\n'
        return code
    elif type == 'memberUnion':
        code: str = ('item = item->getOrInsertChild('
                     'indexStack.back()++, u"{}"_s);\n').format(name)
        code += 'indexStack.push_back(0);\n'
        members: list[Any] = member['members']

        unionMember: dict[str, Any]
        for unionMember in members:
            if 'id' in unionMember:
                unionMember = defs[unionMember['id']]
            unionType: str = unionMember['type']
            code += 'if ({}) {{\n'.format(member_condition(unionMember, defs))
            if unionType.startswith('member'):
                code += define_member(unionMember, defs)
            else:
                unionSig: str = unionMember['signature'].encode('unicode_escape').decode()
                unionName: str = ''
                if 'name' in unionMember:
                    unionName: str = unionMember['name']
                code += 'if (signature == "{}"_ts) {{\n'.format(unionSig)
                code += ('item = item->getOrInsertChild('
                         'indexStack.back()++, "{}"_ts, u"{}"_s);\n').format(
                             unionSig, unionName)
                code += 'indexStack.push_back(0);\n'
                code += define_type(unionMember, defs)
                code += 'item = item->parent();\n'
                code += 'indexStack.pop_back();\n\n'
                code += 'co_await std::suspend_always();\n}\n'
            code += '}\n'
        code += 'item = item->parent();\n'
        code += 'indexStack.pop_back();\n\n'
        return code
    else:
        signature: str = member['signature'].encode('unicode_escape').decode()
        code: str = ''
        if signature == 'VMAD':
            code += 'int ObjectFormat;\n'
        code += ('item = item->getOrInsertChild('
                 'indexStack.back()++, "{}"_ts, u"{}"_s);\n').format(signature, name)
        code += 'indexStack.push_back(0);\n'
        code += 'if (signature == "{}"_ts) {{\n'.format(signature)
        code += define_type(member, defs)
        code += 'co_await std::suspend_always();\n}\n'
        code += 'item = item->parent();\n'
        code += 'indexStack.pop_back();\n\n'
        return code

def define_record(definition: dict[str, Any], defs: dict[str, Any]) -> str:
    signature: str = definition['signature'].encode('unicode_escape').decode()
    if 'id' in definition:
        definition = {**definition, **defs[definition['id']]}

    code: str = (
        'template <>\n'
        'inline ParseTask FormParser<"{}">::parseForm('
        '    DataItem* root, int fileIndex, [[maybe_unused]] bool localized,\n'
        '    [[maybe_unused]] std::span<const std::string> masters,\n'
        '    [[maybe_unused]] const std::string& plugin, const TESFile::Type& signature,\n'
        '    std::istream* const& stream) const\n'
        '{{\n'
        'using ConflictType = DataItem::ConflictType;'
        'DataItem* item = root;\n'
        'std::vector<int> indexStack{{0}};\n'
        '\n').format(signature)
    members: list[Any] = definition['members']
    member: dict[str, Any]
    for member in members:
        if 'id' in member:
            member = {**member, **defs[member['id']]}
            del member['id']
        code += define_member(member, defs)

    code += ('for (;;) {\n'
             'parseUnknown(root, indexStack.front(), fileIndex, signature, *stream);\n'
             'co_await std::suspend_always();\n'
             '}\n')
    code += '}\n\n'
    return code

if __name__ == '__main__':
    dataPath: str = sys.argv[1]
    outPath: str = sys.argv[2]

    code: str = ''
    dataFile: TextIO
    with open(dataPath, 'r') as dataFile:
        data: dict[str, Any] = json.load(dataFile)
        defs: dict[str, Any] = data['defs']

        code += 'namespace TESData\n{\n\n'

        id: str
        definition: dict[str, Any]
        for id, definition in defs.items():
            type: str = definition['type']
            if type == 'record' and 'signature' in definition:
                code += define_record(definition, defs)

        code += '\n}\n'

    outFile: TextIO
    with open(outPath, 'w') as outFile:
        outFile.write(code)
