import { useState, useEffect } from 'react';
import en from '../locales/en';
import zh from '../locales/zh';

// Simple i18n implementation
const locales = { en, zh };
type Locale = 'en' | 'zh';

export const useTranslation = () => {
  // Try to get from localStorage or default to zh (since it's a Chinese project context)
  const [locale, setLocale] = useState<Locale>(() => {
    const saved = localStorage.getItem('locale');
    return (saved === 'en' || saved === 'zh') ? saved : 'zh';
  });

  useEffect(() => {
    localStorage.setItem('locale', locale);
    document.documentElement.lang = locale;
  }, [locale]);

  const t = (key: string) => {
    const keys = key.split('.');
    let value: any = locales[locale];
    for (const k of keys) {
      if (value && typeof value === 'object' && k in value) {
        value = value[k as keyof typeof value];
      } else {
        return key; // Fallback to key if not found
      }
    }
    return value as string;
  };

  return { t, locale, setLocale };
};
