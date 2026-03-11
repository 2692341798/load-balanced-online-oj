import { useRef, useEffect } from 'react'
import Editor, { loader, type OnMount } from '@monaco-editor/react'
import * as monaco from 'monaco-editor'

// Configure Monaco Editor to use local instance
loader.config({ monaco })

interface CodeEditorProps {
  language: string
  value: string
  onChange: (value: string | undefined) => void
  theme?: string
  fontSize?: number
}

const CodeEditor = ({
  language,
  value,
  onChange,
  theme = 'vs-dark',
  fontSize = 14,
}: CodeEditorProps) => {
  const editorRef = useRef<monaco.editor.IStandaloneCodeEditor | null>(null)
  const disposablesRef = useRef<monaco.IDisposable[]>([])

  // Cleanup disposables on unmount
  useEffect(() => {
    return () => {
      disposablesRef.current.forEach((d) => d.dispose())
      disposablesRef.current = []
    }
  }, [])

  const handleEditorDidMount: OnMount = (editor, monacoInstance) => {
    editorRef.current = editor

    // Register snippets for C++
    disposablesRef.current.push(
      monacoInstance.languages.registerCompletionItemProvider('cpp', {
        provideCompletionItems: () => {
          const suggestions = [
            // Basic Structure
            {
              label: 'main',
              kind: monacoInstance.languages.CompletionItemKind.Snippet,
              insertText: [
                '#include <iostream>',
                '#include <vector>',
                '#include <string>',
                '#include <algorithm>',
                '',
                'using namespace std;',
                '',
                'int main() {',
                '    ${1:// code}',
                '    return 0;',
                '}',
              ].join('\n'),
              insertTextRules:
                monacoInstance.languages.CompletionItemInsertTextRule
                  .InsertAsSnippet,
              documentation: 'Basic C++ Main Function',
            },
            // I/O
            {
              label: 'cin',
              kind: monacoInstance.languages.CompletionItemKind.Snippet,
              insertText: 'cin >> ${1:variable};',
              insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
              documentation: 'Standard Input',
            },
            {
              label: 'cout',
              kind: monacoInstance.languages.CompletionItemKind.Snippet,
              insertText: 'cout << ${1:value} << endl;',
              insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
              documentation: 'Standard Output',
            },
            {
                label: 'fastio',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'ios::sync_with_stdio(false);\ncin.tie(nullptr);',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'Fast I/O',
            },
            // Control Flow
            {
              label: 'for',
              kind: monacoInstance.languages.CompletionItemKind.Snippet,
              insertText: [
                'for (int ${1:i} = 0; ${1:i} < ${2:n}; ++${1:i}) {',
                '    ${3:// code}',
                '}',
              ].join('\n'),
              insertTextRules:
                monacoInstance.languages.CompletionItemInsertTextRule
                  .InsertAsSnippet,
              documentation: 'For Loop',
            },
            {
                label: 'while',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: [
                    'while (${1:condition}) {',
                    '    ${2:// code}',
                    '}',
                ].join('\n'),
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'While Loop',
            },
            {
                label: 'if',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: [
                    'if (${1:condition}) {',
                    '    ${2:// code}',
                    '}',
                ].join('\n'),
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'If Statement',
            },
            // STL Containers
            {
                label: 'vector',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'vector<${1:int}> ${2:v};',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'std::vector',
            },
            {
                label: 'map',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'map<${1:key_type}, ${2:value_type}> ${3:m};',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'std::map',
            },
            {
                label: 'set',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'set<${1:type}> ${2:s};',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'std::set',
            },
            {
                label: 'queue',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'queue<${1:type}> ${2:q};',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'std::queue',
            },
            {
                label: 'stack',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'stack<${1:type}> ${2:s};',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'std::stack',
            },
            {
                label: 'pair',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'pair<${1:type1}, ${2:type2}> ${3:p};',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'std::pair',
            },
            // Algorithms
            {
                label: 'sort',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'sort(${1:v}.begin(), ${1:v}.end());',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'std::sort',
            },
            {
                label: 'reverse',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'reverse(${1:v}.begin(), ${1:v}.end());',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'std::reverse',
            },
          ]
          return { suggestions }
        },
      })
    )

    // Register snippets for Java
    disposablesRef.current.push(
      monacoInstance.languages.registerCompletionItemProvider('java', {
        provideCompletionItems: () => {
          const suggestions = [
            // Basic Structure
            {
              label: 'main',
              kind: monacoInstance.languages.CompletionItemKind.Snippet,
              insertText: [
                'import java.util.*;',
                'import java.io.*;',
                '',
                'public class Main {',
                '    public static void main(String[] args) {',
                '        Scanner scanner = new Scanner(System.in);',
                '        ${1:// code}',
                '        scanner.close();',
                '    }',
                '}',
              ].join('\n'),
              insertTextRules:
                monacoInstance.languages.CompletionItemInsertTextRule
                  .InsertAsSnippet,
              documentation: 'Basic Java Main Class',
            },
            // I/O
            {
              label: 'sout',
              kind: monacoInstance.languages.CompletionItemKind.Snippet,
              insertText: 'System.out.println(${1:value});',
              insertTextRules:
                monacoInstance.languages.CompletionItemInsertTextRule
                  .InsertAsSnippet,
              documentation: 'Print Line',
            },
            {
                label: 'scanner',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'Scanner scanner = new Scanner(System.in);',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'Scanner Initialization',
            },
            // Control Flow
            {
                label: 'for',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: [
                    'for (int ${1:i} = 0; ${1:i} < ${2:n}; ${1:i}++) {',
                    '    ${3:// code}',
                    '}',
                ].join('\n'),
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'For Loop',
            },
             {
                label: 'foreach',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: [
                    'for (${1:Type} ${2:item} : ${3:collection}) {',
                    '    ${4:// code}',
                    '}',
                ].join('\n'),
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'Enhanced For Loop',
            },
            {
                label: 'if',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: [
                    'if (${1:condition}) {',
                    '    ${2:// code}',
                    '}',
                ].join('\n'),
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'If Statement',
            },
            // Data Structures
            {
                label: 'list',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'List<${1:Type}> ${2:list} = new ArrayList<>();',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'ArrayList',
            },
            {
                label: 'map',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'Map<${1:KeyType}, ${2:ValueType}> ${3:map} = new HashMap<>();',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'HashMap',
            },
            {
                label: 'set',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'Set<${1:Type}> ${2:set} = new HashSet<>();',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'HashSet',
            },
             // Algorithms
            {
                label: 'sort',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'Arrays.sort(${1:array});',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'Arrays.sort',
            },
          ]
          return { suggestions }
        },
      })
    )

    // Register snippets for Python
    disposablesRef.current.push(
      monacoInstance.languages.registerCompletionItemProvider('python', {
        provideCompletionItems: () => {
          const suggestions = [
            // Functions
            {
              label: 'def',
              kind: monacoInstance.languages.CompletionItemKind.Snippet,
              insertText: [
                'def ${1:function_name}(${2:args}):',
                '\t${3:pass}',
              ].join('\n'),
              insertTextRules:
                monacoInstance.languages.CompletionItemInsertTextRule
                  .InsertAsSnippet,
              documentation: 'Function Definition',
            },
            {
              label: 'ifmain',
              kind: monacoInstance.languages.CompletionItemKind.Snippet,
              insertText: ['if __name__ == "__main__":', '\t${1:main()}'].join(
                '\n'
              ),
              insertTextRules:
                monacoInstance.languages.CompletionItemInsertTextRule
                  .InsertAsSnippet,
              documentation: 'Main Guard',
            },
            // I/O
            {
                label: 'print',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'print(${1:value})',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'Print',
            },
            {
                label: 'input',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'input()',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'Input',
            },
             {
                label: 'map',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: 'map(${1:func}, ${2:iterable})',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'Map',
            },
             {
                label: 'listcomp',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: '[${1:x} for ${1:x} in ${2:iterable}]',
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'List Comprehension',
            },
            // Control Flow
            {
                label: 'for',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: [
                    'for ${1:i} in range(${2:n}):',
                    '\t${3:pass}',
                ].join('\n'),
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'For Loop',
            },
             {
                label: 'if',
                kind: monacoInstance.languages.CompletionItemKind.Snippet,
                insertText: [
                    'if ${1:condition}:',
                    '\t${2:pass}',
                ].join('\n'),
                insertTextRules: monacoInstance.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: 'If Statement',
            },
          ]
          return { suggestions }
        },
      })
    )
  }

  // Update options when fontSize changes
  useEffect(() => {
    if (editorRef.current) {
      editorRef.current.updateOptions({ fontSize })
    }
  }, [fontSize])

  return (
    <Editor
      height="100%"
      language={language}
      theme={theme}
      value={value}
      onChange={onChange}
      onMount={handleEditorDidMount}
      options={{
        minimap: { enabled: false },
        fontSize: fontSize,
        scrollBeyondLastLine: false,
        automaticLayout: true,
        quickSuggestions: true,
        suggestOnTriggerCharacters: true,
        snippetSuggestions: 'inline',
        wordBasedSuggestions: 'currentDocument',
        parameterHints: { enabled: true },
        suggest: {
          showKeywords: true,
          showSnippets: true,
        },
        contextmenu: true,
        // Additional Luogu-like settings
        fontFamily: "'Fira Code', 'Consolas', monospace",
        fontLigatures: true,
        cursorBlinking: 'smooth',
        smoothScrolling: true,
        padding: { top: 16, bottom: 16 },
      }}
    />
  )
}

export default CodeEditor
