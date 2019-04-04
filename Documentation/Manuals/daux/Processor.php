<?php

namespace Todaymade\Daux\Extension;

use League\CommonMark\Environment;
use League\CommonMark;
use League\CommonMark\ElementRendererInterface;
use League\CommonMark\HtmlElement;
use League\CommonMark\Inline\Element\AbstractInline;
use League\CommonMark\Inline\Element\Link;
use Todaymade\Daux\Config;
use Todaymade\Daux\DauxHelper;
use Todaymade\Daux\Exception\LinkNotFoundException;
use Todaymade\Daux\Tree\Entry;

class APIDocLinkParser extends CommonMark\Inline\Parser\AbstractInlineParser
{
    private $compoundLookup;
    private $compoundLookupByRef;
    private $typedefLookup;

    public function __construct($compoundLookup, $typedefLookup) {
        $this->compoundLookup = $compoundLookup;
        $this->typedefLookup = $typedefLookup;

        $this->compoundLookupByRef = array();
        foreach($this->compoundLookup as $key => $value)
            $this->compoundLookupByRef[$value->file] = $value;
    }

    public function getCharacters()
    {
        return ['@'];
    }

    private function getCleanArgList($argList)
    {
        // Strip ( at start and ) at end
        $innerArgList = substr($argList, 1, strlen($argList) - 2);

        // Clean each parameter
        $params = explode(",", $innerArgList);
        $output = array();
        foreach($params as $param)
        {
            // Remove default value if it exists
            $equalsPos = strpos($param, "=");
            if($equalsPos == FALSE)
                $cleanParam = $param;
            else
                $cleanParam = substr($param, 0, $equalsPos);

            // Remove all spaces, we don't need them for comparison
            $cleanParam = $str = trim(preg_replace('/\s+/','', $cleanParam));

            array_push($output, $cleanParam);
        }

        return join(",", $output);
    }

    private function lookUpFunction($typeInfo, $name, $argList, &$functionInfo)
    {
        foreach($typeInfo->functions as $function)
        {
            if($function->name == $name)
            {
                if(empty($argList) or $argList == $this->getCleanArgList($function->arguments))
                {
                    $functionInfo = $function;
                    return TRUE;
                }
            }
        }

        foreach($typeInfo->bases as $base)
        {
            if(array_key_exists($base, $this->compoundLookupByRef))
            {
                $baseTypeInfo = $this->compoundLookupByRef[$base];
                if($this->lookUpFunction($baseTypeInfo, $name, $argList, $functionInfo))
                    return TRUE;
            }
        }

        return FALSE;
    }

    private function lookUpField($typeInfo, $name, &$fieldInfo)
    {
        if(array_key_exists($name, $typeInfo->fields)) {
            $fieldInfo = $typeInfo->fields[$name];
            return TRUE;
        }
        else
        {
            foreach($typeInfo->bases as $base)
            {
                if(array_key_exists($base, $this->compoundLookupByRef))
                {
                    $baseTypeInfo = $this->compoundLookupByRef[$base];
                    if($this->lookUpField($baseTypeInfo, $name,$fieldInfo))
                        return TRUE;
                }
            }
        }

        return FALSE;
    }

    private function lookUpEnum($typeInfo, $name, &$enumInfo)
    {
        if(array_key_exists($name, $typeInfo->enums)) {
            $enumInfo = $typeInfo->enums[$name];
            return TRUE;
        }
        else
        {
            foreach($typeInfo->bases as $base)
            {
                if(array_key_exists($base, $this->compoundLookupByRef))
                {
                    $baseTypeInfo = $this->compoundLookupByRef[$base];
                    if($this->lookUpEnum($baseTypeInfo, $name,$enumInfo))
                        return TRUE;
                }
            }
        }

        return FALSE;
    }

    private function lookUpUnqualifiedEnumValue($typeInfo, $name, &$enumValueInfo)
    {
        foreach($typeInfo->enums as $enum)
        {
            if($enum->isStrong)
                continue;

            foreach($enum->entries as $entry)
            {
                if($entry->name == $name) {
                    $enumValueInfo = $entry;
                    return TRUE;
                }
            }
        }

        foreach($typeInfo->bases as $base)
        {
            if(array_key_exists($base, $this->compoundLookupByRef))
            {
                $baseTypeInfo = $this->compoundLookupByRef[$base];
                if($this->lookUpUnqualifiedEnumValue($baseTypeInfo, $name,$enumValueInfo))
                    return TRUE;
            }
        }

        return FALSE;
    }

    private function lookupCompound($typeName, $checkTypedef)
    {
        if(array_key_exists($typeName, $this->compoundLookup))
            return $this->compoundLookup[$typeName];
        elseif($checkTypedef == true)
        {
            if(array_key_exists($typeName, $this->typedefLookup))
            {
                $typedefRef = $this->typedefLookup[$typeName];
                if(array_key_exists($typedefRef, $this->compoundLookupByRef)) {
                    return $this->compoundLookupByRef[$typedefRef];
                }
            }
        }

        return null;
    }

    private function lookupType($typeName, $argList, &$linkFile, &$linkHash, &$isFunction)
    {
        // Find the file the type is described in
        $linkFile = "";
        $linkHash = "";

        // Check if we're referencing a class/struct/union by doing a direct lookup
        $isFunction = FALSE;

        $typeInfo = $this->lookupCompound($typeName, false);
        if($typeInfo == null)
        {
            // Check with no template parameters
            $typeNameNoT = preg_replace('/<[A-Za-z0-9_,\.\s\(\)]*>/', '', $typeName);
            $typeInfo = $this->lookupCompound($typeNameNoT, false);
        }

        if($typeInfo != null) {
            $linkFile = $typeInfo->file;
            $linkHash = "";
        } else {
            // If not, we're referencing either a member of some class, an enum, or a global
            $names = preg_split('/::/', $typeName);

            if (empty($names)) {
                \Todaymade\Daux\Daux::write(PHP_EOL . "    [WARNING] Invalid type name provided for type link '$typeName'.");
                return FALSE;
            }

            // Cut the name of the member, enum or global and try to find its parent class or namespace
            $memberName0 = array_pop($names);

            // Always remove template parameters from members, we're not using them at the moment
            $memberName0 = preg_replace('/<[A-Za-z0-9_,\.\s\(\)]*>/', '', $memberName0);

            if(empty($names))
                $typeName0 = "";
            else
                $typeName0 = join("::", $names);

            $typeInfo = $this->lookupCompound($typeName0, true);
            if($typeInfo == null)
            {
                // Check with no template parameters
                $typeName0NoT = preg_replace('/<[A-Za-z0-9_,\.\s\(\)]*>/', '', $typeName0);
                $typeInfo = $this->lookupCompound($typeName0NoT, true);
            }

            if($typeInfo != null) {
                // Look for member
                if ($this->lookUpField($typeInfo, $memberName0, $fieldInfo)) {
                    $linkFile = $fieldInfo->file;
                    $linkHash = $fieldInfo->id;
                } else if ($this->lookUpFunction($typeInfo, $memberName0, $argList, $functionInfo)) {
                    $linkFile = $functionInfo->file;
                    $linkHash = $functionInfo->id;

                    $isFunction = TRUE;
                } else if ($this->lookUpEnum($typeInfo, $memberName0, $enumInfo)) {
                    $linkFile = $enumInfo->file;
                    $linkHash = $enumInfo->id;
                } else if ($this->lookUpUnqualifiedEnumValue($typeInfo, $memberName0, $enumValueInfo)) {
                    $linkFile = $enumValueInfo->file;
                    $linkHash = $enumValueInfo->id;
                }
            } else {
                // Cut another name, it could be a qualified enum value
                // Note: We don't need to check depth greater than 2. All nested classes are stored in the top-level
                // compound lookup

                if(!empty($names))
                {
                    $memberName1 = array_pop($names);

                    if(empty($names))
                        $typeName1 = "";
                    else
                        $typeName1 = join("::", $names);

                    if(array_key_exists($typeName1, $this->compoundLookup)) {
                        $typeInfo = $this->compoundLookup[$typeName1];

                        // Look for enum
                        if ($this->lookUpEnum($typeInfo, $memberName1, $enumInfo))
                        {
                            if(array_key_exists($memberName0, $enumInfo->entries))
                            {
                                $enumValueInfo = $enumInfo->entries[$memberName0];

                                $linkFile = $enumValueInfo->file;
                                $linkHash = $enumValueInfo->id;
                            }
                        }
                    }
                }
            }
        }

        return TRUE;
    }

    public function parse(CommonMark\InlineParserContext $inlineContext)
    {
        $cursor = $inlineContext->getCursor();

        // The @ symbol must not have any other characters immediately prior
        $previousChar = $cursor->peek(-1);
        if ($previousChar !== null && $previousChar !== ' ' && $previousChar !== '\t')
            return false;

        // Save the cursor state in case we need to rewind and bail
        $previousState = $cursor->saveState();

        // Advance past the @ symbol
        $cursor->advance();

        // Parse the type name
        $typeName = $cursor->match('/^(?:[A-Za-z0-9_](?:<[A-Za-z0-9_,\.\s\(\)]*>)?)+(?:::(?:[A-Za-z0-9_])+(?:<[A-Za-z0-9_,\.\s\(\)]*>)?)*/');
        if (empty($typeName)) {
            // Regex failed to match
            $cursor->restoreState($previousState);
            return false;
        }

        $argList = $cursor->match('/^\([^\)]*\)/');
        if(!empty($argList)) {
            $argList = $this->getCleanArgList($argList);
        }

        $customTitle = $cursor->match('/^\[[^\]]*\]/');
        if(!empty($customTitle))
            $customTitle = substr($customTitle, 1, strlen($customTitle) - 2);

        // Add bs:: prefix if not specified, and not in some other namespace
        $prefixAdded = false;
        if(substr($typeName, 0, 4) != 'bs::')
        {
            $hasNs = strpos($typeName, '::');

            if($hasNs === FALSE) {
                $typeName = 'bs::' . $typeName;
                $prefixAdded = true;
            }
            else
            {
                $ns = substr($typeName, 0, $hasNs);
                if($ns === 'ct') {
                    $typeName = 'bs::' . $typeName;
                    $prefixAdded = true;
                }
            }
        }

        if(!$this->lookupType($typeName, $argList, $linkFile, $linkHash, $isFunction))
        {
            $cursor->restoreState($previousState);
            return false;
        }

        // Try without the bs:: prefix, if we added it
        if($linkFile === "" and $prefixAdded)
        {
            $typeName = substr($typeName, 4);

            if(!$this->lookupType($typeName, $argList, $linkFile, $linkHash, $isFunction))
            {
                $cursor->restoreState($previousState);
                return false;
            }
        }

        // We cannot find the referenced type or member anywhere, bail.
        if($linkFile === "")
        {
            \Todaymade\Daux\Daux::write(PHP_EOL . "    [WARNING] Unable to find documentation for type link '$typeName'.");

            $cursor->restoreState($previousState);
            return false;
        }

        $linkUrl = 'api:' . $linkFile . ".html";
        if($linkHash !== "")
            $linkUrl .= '#' . $linkHash;

        $readableName = str_replace("bs::","", $typeName);
        if($isFunction)
            $readableName .= '()';

        if(!empty($customTitle))
            $readableName = $customTitle;

        $inlineContext->getContainer()->appendChild(new CommonMark\Inline\Element\Link($linkUrl, $readableName));
        return true;
    }
}

class APIDocLinkRenderer extends CommonMark\Inline\Renderer\LinkRenderer
{
    /**
     * @var Config
     */
    protected $daux;

    public function __construct($daux)
    {
        $this->daux = $daux;
    }

    /**
     * @param string $url
     * @return Entry
     * @throws LinkNotFoundException
     */
    protected function resolveInternalFile($url)
    {
        $triedAbsolute = false;

        // Legacy absolute paths could start with
        // "!" In this case we will try to find
        // the file starting at the root
        if ($url[0] == '!' || $url[0] == '/') {
            $url = ltrim($url, '!/');

            if ($file = DauxHelper::getFile($this->daux['tree'], $url)) {
                return $file;
            }

            $triedAbsolute = true;
        }

        // Seems it's not an absolute path or not found,
        // so we'll continue with the current folder
        if ($file = DauxHelper::getFile($this->daux->getCurrentPage()->getParent(), $url)) {
            return $file;
        }

        // If we didn't already try it, we'll
        // do a pass starting at the root
        if (!$triedAbsolute && $file = DauxHelper::getFile($this->daux['tree'], $url)) {
            return $file;
        }

        throw new LinkNotFoundException("Could not locate file '$url'");
    }

    protected function isValidUrl($url)
    {
        return !empty($url) && $url[0] != '#';
    }

    protected function isExternalUrl($url)
    {
        return preg_match('#^(?:[a-z]+:)?//|^mailto:#', $url);
    }

    protected function isApiUrl($url)
    {
        return preg_match('#^api:#', $url);
    }

    /**
     * @param AbstractInline|Link $inline
     * @param ElementRendererInterface $htmlRenderer
     * @return HtmlElement
     * @throws LinkNotFoundException
     */
    public function render(AbstractInline $inline, ElementRendererInterface $htmlRenderer)
    {
        // This can't be in the method type as
        // the method is an abstract and should
        // have the same interface
        if (!$inline instanceof Link) {
            throw new \RuntimeException(
                'Wrong type passed to ' . __CLASS__ . '::' . __METHOD__ .
                " the expected type was 'League\\CommonMark\\Inline\\Element\\Link' but '" .
                get_class($inline) . "' was provided"
            );
        }

        $element = parent::render($inline, $htmlRenderer);

        $url = $inline->getUrl();

        // empty urls and anchors should
        // not go through the url resolver
        if (!$this->isValidUrl($url)) {
            return $element;
        }

        // Absolute urls, shouldn't either
        if ($this->isExternalUrl($url)) {
            $element->setAttribute('class', 'Link--external');

            return $element;
        }

        if ($this->isApiUrl($url)) {
            $linkPath = substr($url, 4); // Trim 'api:'
            $linkPath = __DIR__ . '/../docs/api/' . $linkPath;

            $pagePath = __DIR__ . '/../docs/' . $this->daux->getCurrentPage()->getUrl();
            $linkPath = DauxHelper::getRelativePath($pagePath, $linkPath);

            $element->setAttribute('href', $linkPath);
            return $element;
        }

        // if there's a hash component in the url, ensure we
        // don't put that part through the resolver.
        $urlAndHash = explode('#', $url);
        $url = $urlAndHash[0];

        try {
            $file = $this->resolveInternalFile($url);
            $url = DauxHelper::getRelativePath($this->daux->getCurrentPage()->getUrl(), $file->getUrl());
        } catch (LinkNotFoundException $e) {
            if ($this->daux->isStatic()) {
                throw $e;
            }

            $element->setAttribute('class', 'Link--broken');
        }

        if (isset($urlAndHash[1])) {
            $url .= '#' . $urlAndHash[1];
        }

        $element->setAttribute('href', $url);

        return $element;
    }
}

class APIDocMemberInfo
{
    public $name = "";
    public $file = "";
    public $id = "";
}

class APIDocFunctionInfo extends APIDocMemberInfo
{
    public $arguments;
}

class APIDocEnumInfo extends APIDocMemberInfo
{
    public $isStrong;
    public $entries;
}

class APIDocCompoundInfo
{
    public $name = "";
    public $file = "";
    public $id = "";
    public $fields = array();
    public $functions = array();
    public $enums = array();
    public $bases = array();
}

class Processor extends \Todaymade\Daux\Processor
{
    private function parsePathAndId($fullId, $isEnumValue)
    {
        $id = substr($fullId, strlen($fullId) - 33);
        $path = substr($fullId, 0, -33);

        if($isEnumValue)
        {
            $parentId = substr($path, strlen($path) - 33);
            $id = $parentId . $id;
            $path = substr($path, 0, -33);
        }

        $separatorPos = strrpos($path, '_');
        $path = substr($path, 0, $separatorPos);

        return array("id"=>(string)$id, "path"=>(string)$path);
    }

    private function parseCommonMember($xmlMember, &$member)
    {
        $idAndPath = $this->parsePathAndId($xmlMember["id"],  $xmlMember->getName() === "enumvalue");

        $member->name = (string)$xmlMember->name;
        $member->file = $idAndPath["path"];
        $member->id = $idAndPath["id"];
    }

    private function parseCompound($xmlCompound)
    {
        $compound = new APIDocCompoundInfo();

        $rawName = (string)$xmlCompound->compoundname;
        $cleanName = htmlspecialchars_decode($rawName); // Convert special HTML characters such as &lt;
        $cleanName = preg_replace('/\s+/', '', $cleanName); // Get rid of whitespace

        $compound->name = $cleanName;
        $compound->file = (string)$xmlCompound["id"];

        // Parse base classes
        foreach($xmlCompound->basecompoundref as $baseClass)
            array_push($compound->bases, (string)$baseClass["refid"]);

        // Parse members
        foreach($xmlCompound->sectiondef as $section)
        {
            foreach($section->memberdef as $member)
            {
                switch($member["kind"])
                {
                    case "variable":
                    case "typedef":
                        $field = new APIDocMemberInfo();
                        $this->parseCommonMember($member, $field);

                        $compound->fields[$field->name] = $field;

                        if($member["kind"] == "typedef")
                            $this->parseTypedef($member, $compound->name . '::' . $field->name);

                        break;
                    case "function":
                        $function = new APIDocFunctionInfo();
                        $this->parseCommonMember($member, $function);

                        $paramStrings = array();
                        foreach($member->param as $param)
                        {
                            // Type node contains mixed content, which isn't supported well using Simple XML. Instead we
                            // hack around it by surrounding raw text in <text> nodes, so they can be iterated over.
                            $cleanTypeXml = preg_replace('~>(.*?)<~', '><text>$1</text><', $param->type->asXML());
                            $cleanTypeXml = str_replace('<text></text>', '', $cleanTypeXml);
                            $typeXml = new \SimpleXMLElement($cleanTypeXml);

                            $paramString = "";
                            foreach($typeXml->children() as $entry) {

                                $numChildren = count($entry->children());
                                if($numChildren == 0)
                                    $paramString .= (string)$entry;
                                else
                                {
                                    foreach($entry->children() as $entry1)
                                        $paramString .= (string)$entry1;
                                }
                            }

                            array_push($paramStrings, $paramString);
                        }

                        $function->arguments = '(' . htmlspecialchars_decode(join(",", $paramStrings)) . ')';

                        array_push($compound->functions, $function);
                        break;
                    case "enum":
                        $enum = new APIDocEnumInfo();
                        $this->parseCommonMember($member, $enum);

                        $enum->isStrong = $member["strong"] === "yes";

                        foreach($member->enumvalue as $xmlEnumValue)
                        {
                            $enumValue = new APIDocMemberInfo();
                            $this->parseCommonMember($xmlEnumValue, $enumValue);

                            $enum->entries[$enumValue->name] = $enumValue;
                        }

                        $compound->enums[$enum->name] = $enum;
                        break;
                }
            }
        }

        return $compound;
    }

    private $compoundLookup;
    private $typedefLookup;
    private function parseCompoundXml($xmlCompoundPath)
    {
        $xml = simplexml_load_file($xmlCompoundPath);

        if($xml === FALSE)
            throw new \RuntimeException("Unable to parse index.xml at path '$xmlCompoundPath'");

        foreach($xml->compounddef as $xmlCompound) {
            $compound = $this->parseCompound($xmlCompound);
            $this->compoundLookup[$compound->name] = $compound;
        }
    }

    private function parseTypedef($xmlMember, $typeName)
    {
        foreach($xmlMember->type->ref as $typeRef)
        {
            if($typeRef["kindref"] == "compound")
            {
                $referencedTypeId = (string)$typeRef["refid"];
                $this->typedefLookup[$typeName] = $referencedTypeId;

                break;
            }
        }
    }

    private function parseFileXml($xmlCompoundPath)
    {
        $xml = simplexml_load_file($xmlCompoundPath);

        if($xml === FALSE)
            throw new \RuntimeException("Unable to parse index.xml at path '$xmlCompoundPath'");

        foreach($xml->compounddef as $xmlCompound) {
            // Top level typedefs and defines
            foreach($xmlCompound->sectiondef as $section)
            {
                foreach($section->memberdef as $member)
                {
                    switch($member["kind"])
                    {
                        case "define":
                        case "typedef":
                            $compound = new APIDocCompoundInfo();
                            $this->parseCommonMember($member, $compound);

                            $this->compoundLookup[$compound->name] = $compound;

                            if($member["kind"] == "typedef")
                                $this->parseTypedef($member, $compound->name);

                            break;
                    }
                }
            }
        }
    }

    public function extendCommonMarkEnvironment(Environment $environment)
    {
        $up = '..' . DIRECTORY_SEPARATOR;
        $xmlRoot = __DIR__ . DIRECTORY_SEPARATOR . $up . $up . 'Generated' . DIRECTORY_SEPARATOR . 'native' . DIRECTORY_SEPARATOR . 'xml' . DIRECTORY_SEPARATOR;
        $xmlPath = $xmlRoot . 'index.xml';
        $xml = simplexml_load_file($xmlPath);

        if($xml === FALSE)
            throw new \RuntimeException("Unable to parse index.xml at path '$xmlPath'");

        foreach($xml->compound as $compound)
        {
            $compoundKind = $compound['kind'];

            switch($compoundKind)
            {
                case "class":
                case "struct":
                case "union":
                case "namespace":
                    $compoundXmlPath = $xmlRoot . $compound["refid"] . '.xml';
                    $this->parseCompoundXml($compoundXmlPath);
                    break;
                case "file":
                    $compoundXmlPath = $xmlRoot . $compound["refid"] . '.xml';
                    $this->parseFileXml($compoundXmlPath);
                    break;
            }
        }

        $environment->addInlineParser(new APIDocLinkParser($this->compoundLookup, $this->typedefLookup));
        $environment->addInlineRenderer('League\CommonMark\Inline\Element\Link', new APIDocLinkRenderer($environment->getConfig('daux')));
    }
}
