from typing import *
import json
import sys

def push(code: TextIO, name: str, signature: Optional[str] = None,
         conflictType: str = 'Override', alignable: bool = True) -> None:
    if signature:
        code.write(
            'pushItem(item, indexStack, "{}"_ts, u"{}"_s, ConflictType::{}, {});\n'
            .format(signature, name, conflictType, 'true' if alignable else 'false'))
    else:
        code.write(
            'pushItem(item, indexStack, u"{}"_s, ConflictType::{}, {});\n'
            .format(name, conflictType, 'true' if alignable else 'false'))

def pop(code: TextIO, name: str, signature: Optional[str] = None) -> None:
    comment: str
    if signature:
        comment = signature + ' - ' + name
    else:
        comment = name
    code.write('popItem(item, indexStack); // {}\n\n'.format(comment))

class FormatValue:
    def divide(code: TextIO, format: dict[str, Any]) -> None:
        value: int = int(format['value'])
        code.write('item->setDisplayData(fileIndex, val.toInt() / {}.0f);\n'
                   .format(value))

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
            code.write('item->setDisplayData(fileIndex, u"{}"_s);\n'.format(label))
            code.write('break;\n')
        code.write('}\n')

    def flags(code: TextIO, format: dict[str, Any]) -> None:
        code.write('item->setDisplayData(fileIndex, u""_s);\n')

        i: int
        bit: str
        name: str
        for i, (bit, name) in enumerate(format['flags'].items()):
            name = name.replace('"', '\\"')
            code.write('item = item->getOrInsertChild({}, u"{}"_s);\n'.format(i, name))
            code.write('if (val.toUInt() & (1U << {})) {{\n'.format(bit))
            code.write('item->setData(fileIndex, u"{}"_s);\n'.format(name))
            code.write('}\n')
            code.write('item = item->parent();\n')

    def ScriptObjectAliasFormat(code: TextIO, format: dict[str, Any]) -> None:
        # TODO
        pass

    def CtdaTypeFormat(code: TextIO, format: dict[str, Any]) -> None:
        # TODO
        pass

    def CTDAFunctionFormat(code: TextIO, format: dict[str, Any]) -> None:
        # TODO
        pass

    def CTDAParam1StringFormat(code: TextIO, format: dict[str, Any]) -> None:
        # TODO
        pass

    def ConditionAliasFormat(code: TextIO, format: dict[str, Any]) -> None:
        # TODO
        pass

    def EventFunctionAndMemberFormat(code: TextIO, format: dict[str, Any]) -> None:
        # TODO
        pass

    def CTDAParam2StringFormat(code: TextIO, format: dict[str, Any]) -> None:
        # TODO
        pass

    def CTDAParam2QuestStageFormat(code: TextIO, format: dict[str, Any]) -> None:
        # TODO
        pass

    # TES4
    def NextObjectIDFormat(code: TextIO, format: dict[str, Any]) -> None:
        # TODO
        pass

    # CLMT
    def ClmtMoonsPhaseLengthFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('const bool masser = (val.toUInt() & 0x40);\n')
        code.write('const bool secunda = (val.toUInt() & 0x80);\n')
        code.write('const QString moon = '
                   'masser && secunda ? u"Masser, Secunda"_s '
                   ': masser ? u"Masser"_s '
                   ': secunda ? u"Secunda"_s '
                   ': u"No Moon"_s;\n')
        code.write('const int phaseLength = (val.toUInt() & 0x3F);\n')
        code.write('item->setDisplayData(fileIndex, u"%1 / %2"_s'
                   '.arg(moon).arg(phaseLength));\n')

    # CLMT
    def ClmtTimeFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('const int hours = val.toUInt() / 6;\n')
        code.write('const int minutes = (val.toUInt() % 6) * 10;\n')
        code.write('item->setDisplayData(fileIndex, u"%1:%2"_s'
                   '.arg(hours, 2, 10, QChar(u\'0\'))'
                   '.arg(minutes, 2, 10, QChar(u\'0\')));\n')

    # LAND
    def AtxtPositionFormat(code: TextIO, format: dict[str, Any]) -> None:
        pass

    # NAVM
    def Vertex0Format(code: TextIO, format: dict[str, Any]) -> None:
        pass

    def Vertex1Format(code: TextIO, format: dict[str, Any]) -> None:
        pass

    def Vertex2Format(code: TextIO, format: dict[str, Any]) -> None:
        pass

    def Edge0Format(code: TextIO, format: dict[str, Any]) -> None:
        pass

    def Edge1Format(code: TextIO, format: dict[str, Any]) -> None:
        pass

    def Edge2Format(code: TextIO, format: dict[str, Any]) -> None:
        pass

    # NPC_
    def TintLayerFormat(code: TextIO, format: dict[str, Any]) -> None:
        pass

    # PACK / PLDT / PLVD
    def PackageLocationAliasFormat(code: TextIO, format: dict[str, Any]) -> None:
        pass

    # PERK
    def PerkDATAQuestStageFormat(code: TextIO, format: dict[str, Any]) -> None:
        pass

    # PERK
    def EPFDActorValueFormat(code: TextIO, format: dict[str, Any]) -> None:
        # TODO: lookup actor value
        pass

    # QUST
    def QuestAliasFormat(code: TextIO, format: dict[str, Any]) -> None:
        pass

    # QUST
    def QuestExternalAliasFormat(code: TextIO, format: dict[str, Any]) -> None:
        pass

    # REFR
    def REFRNavmeshTriangleFormat(code: TextIO, format: dict[str, Any]) -> None:
        pass

    # REGN
    def HideFFFF_Format(code: TextIO, format: dict[str, Any]) -> None:
        code.write('if (val.toUInt() == 0xFFFF) {\n'
                   'item->setDisplayData(fileIndex, u""_s);\n'
                   '}\n')

    # WTHR
    def CloudSpeedFormat(code: TextIO, format: dict[str, Any]) -> None:
        code.write('item->setDisplayData(fileIndex, (val.toInt() - 127) / 1270.0f);\n')

def format_val(code: TextIO, format: dict[str, Any], defs: dict[str, Any]) -> None:
    if 'id' in format:
        format = defs[format['id']]

    type: str = format['type']
    getattr(FormatValue, type)(code, format)

class UnionDecider:
    def CTDACompValueDecider(code: TextIO) -> None:
        code.write('decider = (item->parent()->childData('
                   'u"Type"_s, fileIndex).toInt() & 0x04) != 0;\n')

    def CTDAParam1Decider(code: TextIO) -> None:
        # TODO: Analyze functions from exe
        code.write('decider = 0;\n')

    def CTDAParam2Decider(code: TextIO) -> None:
        # TODO: Analyze functions from exe
        code.write('decider = 0;\n')

    def CTDAParam2VATSValueParamDecider(code: TextIO) -> None:
        # TODO: Analyze functions from exe
        code.write('decider = 4;\n')

    def CTDAReferenceDecider(code: TextIO) -> None:
        # TODO: Analyze functions from exe
        code.write('decider = 0;\n')

    def ScriptPropertyDecider(code: TextIO) -> None:
        code.write('decider = '
                   'item->parent()->childData(u"Type"_s, fileIndex).toInt();\n')

    def ScriptObjFormatDecider(code: TextIO) -> None:
        code.write('decider = ObjectFormat == 1;\n')

    # BOOK
    def BOOKTeachesDecider(code: TextIO) -> None:
        code.write('const int flags = '
                   'item->parent()->childData(u"Flags"_s, fileIndex).toInt();\n')
        code.write('decider = (flags & 0x04) ? 1 : 0;\n')

    # FACT / PACK
    def TypeDecider(code: TextIO) -> None:
        code.write('decider = '
                   'item->parent()->childData(u"Type"_s, fileIndex).toInt();\n')

    # GMST
    def GMSTUnionDecider(code: TextIO) -> None:
        code.write('switch (root->childData("EDID"_ts, fileIndex)'
                   '.toString()[0].unicode()) {\n'
                   "case u'b': decider = 3; break;\n"
                   "case u'f': decider = 2; break;\n"
                   "case u'i': decider = 1; break;\n"
                   "case u'u': decider = 1; break;\n"
                   "case u's': decider = 0; break;\n"
                   '}\n')

    # LVLN
    def COEDOwnerDecider(code: TextIO) -> None:
        # TODO: Resolve "Owner" form type (NPC_ -> 0, FACT -> 1)
        code.write('decider = 0;\n')

    # MGEF
    def MGEFAssocItemDecider(code: TextIO) -> None:
        # TODO: Move streampos +0x38 to read "Archtype" early and map to assoc type
        code.write('decider = 0;\n')

    # NAVI
    def NAVIIslandDataDecider(code: TextIO) -> None:
        code.write('decider = '
                   'item->parent()->childData(u"Is Island"_s, fileIndex).toBool();\n')

    # NAVI
    def NAVIParentDecider(code: TextIO) -> None:
        code.write('const auto worldspace = item->parent()->childData('
                   'u"Parent Worldspace"_s, fileIndex).toUInt();\n')
        code.write('decider = worldspace == 0x0000003CU ? 0 : 1;\n')

    # NAVM
    def NVNMParentDecider(code: TextIO) -> None:
        code.write('const auto worldspace = item->parent()->childData('
                   'u"Parent Worldspace"_s, fileIndex).toUInt();\n')
        code.write('decider = worldspace ? 0 : 1;\n')

    # NPC_
    def NPCLevelDecider(code: TextIO) -> None:
        code.write('const int flags = '
                   'item->parent()->childData(u"Flags"_s, fileIndex).toInt();\n')
        code.write('decider = (flags & 0x80) ? 1 : 0;\n')

    # PACK
    def PubPackCNAMDecider(code: TextIO) -> None:
        code.write('const QString activityType = '
                   'item->parent()->childData("ANAM"_ts, fileIndex).toString();\n'
                   'if (activityType == u"Bool"_s) {\n'
                   'decider = 1;\n'
                   '} else if (activityType == u"Int"_s) {\n'
                   'decider = 2;\n'
                   '} else if (activityType == u"Float"_s) {\n'
                   'decider = 3;\n'
                   '} else {\n'
                   'decider = 0;\n'
                   '}\n')

    # PERK
    def PerkDATADecider(code: TextIO) -> None:
        code.write('decider = item->parent()->findChild("PRKE"_ts)'
                   '->childData(u"Type"_s, fileIndex).toInt();\n')

    # PERK
    def EPFDDecider(code: TextIO) -> None:
        code.write('decider = item->parent()->childData('
                   '"EPFT"_ts, fileIndex).toInt();\n')

class DefineType:
    def __integral(type: str, code: TextIO, element: dict[str, Any]) -> None:
        code.write('QVariant val = TESFile::readType<{}>(*stream);\n'.format(type))
        if element.get('name') == 'Object Format':
            code.write('ObjectFormat = val.toInt();\n')
        code.write('item->setData(fileIndex, val);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)

    def int0(code: TextIO, element: dict[str, Any]) -> None:
        code.write('QVariant val = 0;\n')
        code.write('item->setData(fileIndex, val);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)

    def int8(code: TextIO, element: dict[str, Any]) -> None:
        DefineType.__integral('std::int8_t', code, element)

    def int16(code: TextIO, element: dict[str, Any]) -> None:
        DefineType.__integral('std::int16_t', code, element)

    def int32(code: TextIO, element: dict[str, Any]) -> None:
        DefineType.__integral('std::int32_t', code, element)

    def uint8(code: TextIO, element: dict[str, Any]) -> None:
        DefineType.__integral('std::uint8_t', code, element)

    def uint16(code: TextIO, element: dict[str, Any]) -> None:
        DefineType.__integral('std::uint16_t', code, element)

    def uint32(code: TextIO, element: dict[str, Any]) -> None:
        DefineType.__integral('std::uint32_t', code, element)

    def float(code: TextIO, element: dict[str, Any]) -> None:
        code.write('QVariant val = TESFile::readType<float>(*stream);\n')
        code.write('item->setData(fileIndex, val);\n')

    def string(code: TextIO, element: dict[str, Any]) -> None:
        localized: bool = element.get('localized', False)
        if localized:
            code.write('item->setData('
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
            code.write(
                'item->setData(fileIndex, readWstring<{}>(*stream));\n'
                .format(lengthType))
        else:
            code.write(
                'item->setData(fileIndex, readZstring(*stream));\n')

    def formId(code: TextIO, element: dict[str, Any]) -> None:
        code.write('item->setData('
                   'fileIndex, readFormId(masters, plugin, *stream));\n')

    def bytes(code: TextIO, element: dict[str, Any]) -> None:
        size: int = element.get('size', 256)
        code.write('item->setData(fileIndex, readBytes(*stream, {}));\n'.format(
            size))

    def array(code: TextIO, element: dict[str, Any]) -> None:
        name: str = element['name']
        alignable: bool = True
        if 'defFlags' in element:
            defFlags: list[str] = element['defFlags']
            if 'notAlignable' in defFlags:
                alignable = False

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
        push(code, elementName, alignable=alignable)
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
                structElement = defs[structElement['id']] | structElement
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
        code.write('[[maybe_unused]] int decider = 0;\n')
        getattr(UnionDecider, decider)(code)

        unionElements: list[Any] = element['elements']
        code.write('switch (decider) {\n')

        num: int
        unionElement: dict[str, Any]
        for num, unionElement in enumerate(unionElements):
            if 'id' in unionElement:
                unionElement = defs[unionElement['id']]

            code.write('case {}: {{\n'.format(num))
            define_type(code, unionElement, defs)
            code.write('} break;\n')
        code.write('}\n')

    def empty(code: TextIO, element: dict[str, Any]) -> None:
        code.write('// TODO: empty\n')

    def memberStruct(code: TextIO, element: dict[str, Any]) -> None:
        members: list[Any] = element['members']
        structMember: dict[str, Any]
        for structMember in members:
            if 'id' in structMember:
                structMember = defs[structMember['id']] | structMember
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
                structMember = defs[structMember['id']] | structMember
                del structMember['id']
            structType: str = structMember['type']
            if structType.startswith('member'):
                define_member(code, structMember, defs)
            else:
                structSig: str = structMember['signature'].encode('unicode_escape').decode()
                structName: str = ''
                if 'name' in structMember:
                    structName: str = structMember['name']
                push(code, structName, signature=structSig, conflictType=conflictType)
                code.write('if (signature == "{}"_ts) {{\n'.format(structSig))
                define_type(code, structMember, defs)
                code.write('co_await std::suspend_always();\n}\n')
                pop(code, structName, signature=structSig)
        pop(code, name)

    def memberUnion(code: TextIO, member: dict[str, Any], defs: dict[str, Any]) -> None:
        name: str = member.get('name', 'Unknown')
        conflictType: str = member.get('conflictType', 'Override')

        push(code, name, conflictType=conflictType)
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
        push(code, name, signature=signature, conflictType=conflictType)
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
        definition = defs[definition['id']] | definition

    code.write(
        'template <>\n'
        'void FormParser<"{}">::parseFlags(DataItem* root, [[maybe_unused]] int fileIndex, \n'
        '    [[maybe_unused]] std::uint32_t flags) const\n'
        '{{\n'
        '[[maybe_unused]] DataItem* item = root->getOrInsertChild(0, u"Record Flags"_s);\n'
        '\n'.format(signature))

    if 'flags' in definition:
        flags: dict[str, Any] = definition['flags']

        flagsDict: dict[str, str] = {}
        flagsType: str = flags['type']
        if flagsType == 'flags':
            flagsDict = flags['flags']
        elif flagsType == 'formatUnion':
            formats: list[Any] = flags['formats']

            format: dict[str, Any]
            for format in formats:
                flagsDict |= format['flags']
        else:
            code.write('#pragma message(warning: unknown flags type {}'.format(flagsType))

        i: int
        bit: str
        name: str
        for i, (bit, name) in enumerate(flagsDict.items()):
            code.write('item = item->getOrInsertChild({}, u"{}"_s);\n'.format(i, name))
            code.write('if (flags & (1U << {})) {{\n'.format(bit))
            code.write('item->setData(fileIndex, u"{}"_s);\n'.format(name))
            code.write('}\n')
            code.write('item = item->parent();\n')

    code.write('}\n\n')

    code.write(
        'template <>\n'
        'ParseTask FormParser<"{}">::parseForm('
        '    DataItem* root, int fileIndex, [[maybe_unused]] bool localized,\n'
        '    [[maybe_unused]] std::span<const std::string> masters,\n'
        '    [[maybe_unused]] const std::string& plugin, const TESFile::Type& signature,\n'
        '    std::istream* const& stream) const\n'
        '{{\n'
        'using ConflictType = DataItem::ConflictType;'
        'DataItem* item = root;\n'
        'std::vector<int> indexStack{{1}};\n'
        '\n'.format(signature))

    members: list[Any] = definition['members']

    member: dict[str, Any]
    for member in members:
        if 'id' in member:
            member = defs[member['id']] | member
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

        outFile: TextIO
        with open(outPath, 'w') as outFile:
            outFile.write('namespace TESData\n{\n\n')

            id: str
            definition: dict[str, Any]
            for id, definition in defs.items():
                type: str = definition['type']
                if type == 'record' and 'signature' in definition:
                    define_record(outFile, definition, defs)

            outFile.write('\n}\n')
