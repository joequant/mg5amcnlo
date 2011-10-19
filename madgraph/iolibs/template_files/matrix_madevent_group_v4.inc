SUBROUTINE SMATRIX%(proc_id)s(P,ANS)
C  
%(info_lines)s
C 
C MadGraph for Madevent Version
C 
C Returns amplitude squared summed/avg over colors
c and helicities
c for the point in phase space P(0:3,NEXTERNAL)
C  
%(process_lines)s
C  
    IMPLICIT NONE
C  
C CONSTANTS
C  
    Include 'genps.inc'
    Include 'maxconfigs.inc'
    Include 'nexternal.inc'
    Include 'maxamps.inc'
    INTEGER                 NCOMB         
    PARAMETER (             NCOMB=%(ncomb)d)
    INTEGER    NGRAPHS
    PARAMETER (NGRAPHS=%(ngraphs)d) 
    INTEGER    NDIAGS
    PARAMETER (NDIAGS=%(ndiags)d) 
    INTEGER    THEL
    PARAMETER (THEL=2*NCOMB)
C  
C ARGUMENTS 
C  
    REAL*8 P(0:3,NEXTERNAL),ANS
C  
C LOCAL VARIABLES 
C  
        INTEGER NHEL(NEXTERNAL,NCOMB),NTRY(2)
        INTEGER ISHEL(2)
        REAL*8 T,MATRIX%(proc_id)s
        REAL*8 R,SUMHEL,TS(NCOMB)
        INTEGER I,IDEN
        INTEGER JC(NEXTERNAL),II
        LOGICAL GOODHEL(NCOMB,2)
        REAL*8 HWGT, XTOT, XTRY, XREJ, XR, YFRAC(0:NCOMB)
        INTEGER IDUM, NGOOD(2), IGOOD(NCOMB,2)
	INTEGER JHEL(2), J, JJ
        REAL     XRAN1
        EXTERNAL XRAN1
C  
C GLOBAL VARIABLES
C  
    DOUBLE PRECISION AMP2(MAXAMPS), JAMP2(0:MAXFLOW)
    COMMON/TO_AMPS/  AMP2,       JAMP2
    
    CHARACTER*101         HEL_BUFF
    COMMON/TO_HELICITY/  HEL_BUFF
    
    INTEGER IMIRROR
    COMMON/TO_MIRROR/ IMIRROR
 
    REAL*8 POL(2)
    COMMON/TO_POLARIZATION/ POL
    
    INTEGER          ISUM_HEL
    LOGICAL                    MULTI_CHANNEL
    COMMON/TO_MATRIX/ISUM_HEL, MULTI_CHANNEL
%(define_iconfigs_lines)s
    DATA IDUM /0/
    DATA XTRY, XREJ /0,0/
    DATA NTRY /0,0/
    DATA NGOOD /0,0/
    DATA ISHEL/0,0/
    SAVE YFRAC, IGOOD, JHEL
    DATA GOODHEL/THEL*.FALSE./
%(helicity_lines)s
%(den_factor_line)s
C ----------
C BEGIN CODE
C ----------
    NTRY(IMIRROR)=NTRY(IMIRROR)+1
    DO I=1,NEXTERNAL
       JC(I) = +1
    ENDDO
     
    IF (multi_channel) THEN
        DO I=1,NDIAGS
            AMP2(I)=0D0
        ENDDO
        JAMP2(0)=%(ncolor)d
        DO I=1,INT(JAMP2(0))
            JAMP2(I)=0D0
        ENDDO
    ENDIF
    ANS = 0D0
    WRITE(HEL_BUFF,'(20I5)') (0,I=1,NEXTERNAL)
    DO I=1,NCOMB
       TS(I)=0d0
    ENDDO
    IF (ISHEL(IMIRROR) .EQ. 0 .OR. NTRY(IMIRROR) .LE. MAXTRIES) THEN
      DO I=1,NCOMB
         IF (GOODHEL(I,IMIRROR) .OR. NTRY(IMIRROR).LE.MAXTRIES) THEN
            T=MATRIX%(proc_id)s(P ,NHEL(1,I),JC(1))
            DO JJ=1,nincoming
               IF(POL(JJ).NE.1d0.AND.NHEL(JJ,I).EQ.INT(SIGN(1d0,POL(JJ)))) THEN
                  T=T*ABS(POL(JJ))
               ELSE IF(POL(JJ).NE.1d0)THEN
                  T=T*(2d0-ABS(POL(JJ)))
               ENDIF
            ENDDO
            ANS=ANS+T
            TS(I)=T
         ENDIF
      ENDDO
      JHEL(IMIRROR) = 1
      IF(NTRY(IMIRROR).LE.MAXTRIES)THEN
         DO I=1,NCOMB
            IF (.NOT.GOODHEL(I,IMIRROR) .AND. (TS(I).GT.ANS*LIMHEL/NCOMB)) THEN
               GOODHEL(I,IMIRROR)=.TRUE.
               NGOOD(IMIRROR) = NGOOD(IMIRROR) +1
               IGOOD(NGOOD(IMIRROR),IMIRROR) = I
               PRINT *,'Added good helicity ',I,TS(I)*NCOMB/ANS
            ENDIF
         ENDDO
      ENDIF
        IF(NTRY(IMIRROR).EQ.MAXTRIES)THEN
           ISHEL(IMIRROR)=MIN(ISUM_HEL,NGOOD(IMIRROR))
        ENDIF
    ELSE              !LOOP OVER GOOD HELICITIES
        DO J=1,ISHEL(IMIRROR)
            JHEL(IMIRROR)=JHEL(IMIRROR)+1
            IF (JHEL(IMIRROR) .GT. NGOOD(IMIRROR)) JHEL(IMIRROR)=1
            HWGT = REAL(NGOOD(IMIRROR))/REAL(ISHEL(IMIRROR))
            I = IGOOD(JHEL(IMIRROR),IMIRROR)
            T=MATRIX%(proc_id)s(P ,NHEL(1,I),JC(1))            
            DO JJ=1,nincoming
              IF(POL(JJ).NE.1d0.AND.NHEL(JJ,I).EQ.INT(SIGN(1d0,POL(JJ)))) THEN
                T=T*ABS(POL(JJ))
              ELSE IF(POL(JJ).NE.1d0)THEN
                T=T*(2d0-ABS(POL(JJ)))
              ENDIF
            ENDDO
            ANS=ANS+T*HWGT
            TS(I)=T*HWGT
        ENDDO
        IF (ISHEL(IMIRROR) .EQ. 1) THEN
          WRITE(HEL_BUFF,'(20i5)')(NHEL(II,I),II=1,NEXTERNAL)
        ENDIF
    ENDIF
    IF (ISHEL(IMIRROR) .NE. 1) THEN
    R=XRAN1(IDUM)*ANS
    SUMHEL=0d0
    DO I=1,NCOMB
       SUMHEL=SUMHEL+TS(I)
       IF(R.LT.SUMHEL)THEN
          WRITE(HEL_BUFF,'(20i5)')(NHEL(II,I),II=1,NEXTERNAL)
          GOTO 10
       ENDIF
    ENDDO
 10 CONTINUE   
    ENDIF
    IF (MULTI_CHANNEL) THEN
        XTOT=0D0
        DO I=1,NDIAGS
            XTOT=XTOT+AMP2(I)
        ENDDO
        IF (XTOT.NE.0D0) THEN
%(set_amp2_line)s
        ELSE
            ANS=0D0
        ENDIF
    ENDIF
    ANS=ANS/DBLE(IDEN)
    END
 
 
REAL*8 FUNCTION MATRIX%(proc_id)s(P,NHEL,IC)
C  
%(info_lines)s
C
C Returns amplitude squared summed/avg over colors
c for the point with external lines W(0:6,NEXTERNAL)
C  
%(process_lines)s
C  
    IMPLICIT NONE
C  
C CONSTANTS
C  
    INTEGER    NGRAPHS
    PARAMETER (NGRAPHS=%(ngraphs)d) 
    include 'genps.inc'
    include 'nexternal.inc'
    include 'maxamps.inc'
    INTEGER    NWAVEFUNCS,     NCOLOR
    PARAMETER (NWAVEFUNCS=%(nwavefuncs)d, NCOLOR=%(ncolor)d) 
    REAL*8     ZERO
    PARAMETER (ZERO=0D0)
    COMPLEX*16 IMAG1
    PARAMETER (IMAG1=(0D0,1D0))
C  
C ARGUMENTS 
C  
    REAL*8 P(0:3,NEXTERNAL)
    INTEGER NHEL(NEXTERNAL), IC(NEXTERNAL)
C  
C LOCAL VARIABLES 
C  
    INTEGER I,J
    COMPLEX*16 ZTEMP
    REAL*8 DENOM(NCOLOR), CF(NCOLOR,NCOLOR)
    COMPLEX*16 AMP(NGRAPHS), JAMP(NCOLOR)
    COMPLEX*16 W(%(wavefunctionsize)d,NWAVEFUNCS)
C  
C GLOBAL VARIABLES
C  
    Double Precision amp2(maxamps), jamp2(0:maxflow)
    common/to_amps/  amp2,       jamp2
    include 'coupl.inc'
C  
C COLOR DATA
C  
%(color_data_lines)s
C ----------
C BEGIN CODE
C ----------
%(helas_calls)s
%(jamp_lines)s
    MATRIX%(proc_id)s = 0.D0 
    DO I = 1, NCOLOR
        ZTEMP = (0.D0,0.D0)
        DO J = 1, NCOLOR
            ZTEMP = ZTEMP + CF(J,I)*JAMP(J)
        ENDDO
        MATRIX%(proc_id)s=MATRIX%(proc_id)s+ZTEMP*DCONJG(JAMP(I))/DENOM(I)   
    ENDDO
%(amp2_lines)s
    Do I = 1, NCOLOR
        Jamp2(i)=Jamp2(i)+Jamp(i)*dconjg(Jamp(i))
    Enddo

    END