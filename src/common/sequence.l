;;; sequencep - sequence predicate 
(defun sequencep (sequence)
  (or (vectorp sequence) (listp sequence)))

;;; reverse - reverse sequence
(defun reverse (src)
  (cond
   ((listp src) (mu:foldl (:lambda (x r) (cons r x)) () src))
   ((vectorp src) (list-to-vector (reverse (vector-to-list src))))
   (:t (raise "must be a sequence (reverse)" src))))

;;; reduce - reduce sequence
(defun reduce (fn src init)
  (check-type fn :func "is not a function (reverse)")
  (cond
   ((listp src) (mu:foldl fn init src))
   ((vectorp src) (list-to-vector (reduce fn (vector-to-list src) init)))
   (:t (raise "must be a sequence (reduce)" src))))

;;; concatenate - concatenate sequences
(defun concatenate (type :rest sequences)
  (let* ((cat-str (open-output-string ""))
         (concat (:lambda (src)
                   (cond
                    ((eq type :string)
                     (cond
                      ((stringp src) (mu:vector-mapc (:lambda (el) (fmt cat-str "~A" el)) src))
                      ((listp src) (mu:mapc
                                    (:lambda (el)
                                             (if (charp el)
                                                 (fmt cat-str "~A" el)
                                               (raise "must be a character (concatenate)" el)))
                                             src))
                      ((vectorp src) (mu:vector-mapc
                                      (:lambda (el)
                                               (if (charp el)
                                                   (fmt cat-str "~A" el)
                                                 (raise "must be a character (concatenate)" el)))
                                      src))
                      (:t (raise "must be a character sequence (concatenate)" src))))
                     ((eq type :vector)
                      (cond
                       ((vectorp src) (mu:vector-mapc (:lambda (el) (fmt cat-str "~A " el)) src))
                       ((listp src) (mu:mapc (:lambda (el) (fmt cat-str "~A " el)) src))
                       (:t (raise "must be a sequence (concatenate)" src))))
                     ((eq type :list)
                      (cond
                       ((vectorp src) (mu:vector-mapc (:lambda (el) (fmt cat-str "~A " el)) src))
                       ((listp src) (mu:mapc (:lambda (el) (fmt cat-str "~A " el)) src))
                       (:t (raise "must be a sequence (concatenate)" src))))))))
    (cond
     ((eq type :list)
      (fmt cat-str "(")
      (mu:mapc concat sequences)
      (fmt cat-str ")"))
     ((eq type :vector)
      (fmt cat-str "#(:t ")
      (mu:mapc concat sequences)
      (fmt cat-str ")"))
     ((eq type :string)
      (fmt cat-str "\"")
      (mu:mapc concat sequences)
      (fmt cat-str "\""))
     (:t (raise "is not a sequence type (concatenate)" type)))
    (mu:read (open-input-string (get-output-stream-string cat-str)))))

;;; count-if (inefficient for very large sequences) 
(defun count-if (predicate sequence)
  (check-type predicate :func "is not a function (count-if)")
  (typecase sequence
    (:vector (count-if predicate (vector-to-list sequence)))
    (:cons
     (mu:foldl 
      (:lambda (acc el) (if (predicate el) (1+ acc) acc))
      0
      sequence))
    (:t (raise "is not a sequence (count-if)" sequence))))

;;; length - length of sequence
(defun common:length (sequence)
  (typecase sequence
    (:null 0)
    (:string (mu:vector-length sequence))        
    (:vector (mu:vector-length sequence))
    (:cons (mu:length sequence))
    (:t (raise "is not a sequence (length)" sequence))))

;;; elt - sequence ref
(defun elt (index sequence)
  (check-type index :fixnum "is not a fixnum (elt)")
  (when (fixnum< index 0) (raise "is a negative fixnum (elt)" index))
  (typecase sequence
    (:vector
     (unless (fixnum< index (mu:vector-length sequence))
       (raise "index out of range (elt)" index))
     (mu:vector-ref sequence index))
    (:cons
     (unless (fixnum< index (mu:length sequence))
       (raise "index out of range (elt)" index))
     (nth index sequence))
    (:t (raise "is not a sequence (elt)" sequence))))

;;; find-if predicate sequence
(defun find-if (predicate sequence)
  (check-type predicate :func "is not a function (find-if)")
  (block :nil
    (cond
      ((vectorp sequence)
       (mu:vector-mapc
        (:lambda (el) (when (predicate el) (return el)))
        sequence))
      ((listp sequence)
       (mu:mapc
        (:lambda (el) (when (predicate el) (return el)))
          sequence))
      (:t (raise "is not a sequence (find-if)" sequence)))
    :nil))

;;; find-if* predicate sequence...
(defun find-if* (predicate :rest sequences)
  (check-type predicate :func "is not a function (find-in)")
  (block :nil
    (mu:mapc
     (:lambda (sequence)
       (cond
        ((vectorp sequence)
         (mu:vector-mapc
          (:lambda (el) (when (predicate el) (return el)))
          sequence))
        ((listp sequence)
         (mu:mapc
          (:lambda (el) (when (predicate el) (return el)))
          sequence))
        (:t (raise "is not a sequence (find-if)" sequence)))
       sequences))
    :nil))
