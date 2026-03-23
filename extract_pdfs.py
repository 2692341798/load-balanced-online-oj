import fitz
import os

pdf_dir = "/Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/article"
pdf_files = [f for f in os.listdir(pdf_dir) if f.endswith('.pdf')]

for pdf_file in pdf_files:
    try:
        doc = fitz.open(os.path.join(pdf_dir, pdf_file))
        text = ""
        # Extract first 20 pages
        for i in range(min(20, len(doc))):
            text += doc[i].get_text() + "\n"
        
        with open(f"{pdf_file}.txt", "w", encoding="utf-8") as f:
            f.write(text)
    except Exception as e:
        pass
